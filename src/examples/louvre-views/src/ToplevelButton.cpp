#include "Toplevel.h"
#include "ToplevelButton.h"
#include "ToplevelView.h"
#include "Global.h"
#include "InputRect.h"
#include "Pointer.h"
#include "src/Compositor.h"
#include <LKeyboard.h>
#include <LSeat.h>
#include <LCursor.h>

ToplevelButton::ToplevelButton(LView *parent, ToplevelView *toplevelView, ButtonType type) : LTextureView(nullptr, parent)
{
    this->toplevelView = toplevelView;
    this->buttonType = type;
    setBufferScale(2);
    enableInput(true);
    update();
}

void ToplevelButton::update()
{
    if (buttonType == Close)
    {
        if (toplevelView->buttonsContainer->pointerIsOver())
        {
            if (pressed)
                G::setTexViewConf(this, G::CloseButtonEnabledPressed);
            else
                G::setTexViewConf(this, G::CloseButtonEnabledHover);
        }
        else
        {
            if (toplevelView->toplevel->activated())
                G::setTexViewConf(this, G::CloseButtonEnabled);
            else
                G::setTexViewConf(this, G::ButtonDisabled);
        }
    }
    else if (buttonType == Minimize)
    {
        if (toplevelView->toplevel->fullscreen())
        {
            G::setTexViewConf(this, G::ButtonDisabled);
        }
        else
        {
            if (toplevelView->buttonsContainer->pointerIsOver())
            {
                if (pressed)
                    G::setTexViewConf(this, G::MinimizeButtonEnabledPressed);
                else
                    G::setTexViewConf(this, G::MinimizeButtonEnabledHover);
            }
            else
            {
                if (toplevelView->toplevel->activated())
                    G::setTexViewConf(this, G::MinimizeButtonEnabled);
                else
                    G::setTexViewConf(this, G::ButtonDisabled);
            }
        }
    }
    else
    {

        bool altMode = !seat()->keyboard()->isKeyCodePressed(KEY_LEFTALT) || toplevelView->toplevel->fullscreen();

        if (toplevelView->buttonsContainer->pointerIsOver())
        {
            if (pressed)
            {
                if (altMode)
                {
                    if (toplevelView->toplevel->fullscreen())
                        G::setTexViewConf(this, G::UnfullscreenButtonEnabledPressed);
                    else
                        G::setTexViewConf(this, G::FullscreenButtonEnabledPressed);
                }
                else
                    G::setTexViewConf(this, G::MaximizeButtonEnabledPressed);
            }
            else
            {
                if (altMode)
                {
                    if (toplevelView->toplevel->fullscreen())
                        G::setTexViewConf(this, G::UnfullscreenButtonEnabledHover);
                    else
                        G::setTexViewConf(this, G::FullscreenButtonEnabledHover);
                }
                else
                    G::setTexViewConf(this, G::MaximizeButtonEnabledHover);
            }
        }
        else
        {
            if (toplevelView->toplevel->activated())
                G::setTexViewConf(this, G::MaximizeButtonEnabled);
            else
                G::setTexViewConf(this, G::ButtonDisabled);
        }
    }
}

void ToplevelButton::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    if (button != LPointer::Button::Left)
        return;

    if (pressed && state == LPointer::Released)
    {
        if (buttonType == Close)
        {
            toplevelView->toplevel->close();
        }
        else if (buttonType == Minimize)
        {
            if (!toplevelView->toplevel->fullscreen())
                toplevelView->toplevel->setMinimizedRequest();
        }
        else
        {
            if (toplevelView->toplevel->fullscreen())
            {
                toplevelView->toplevel->unsetFullscreenRequest();
            }
            else
            {
                bool altMode = seat()->keyboard()->isKeyCodePressed(KEY_LEFTALT);

                if (!altMode)
                {
                    toplevelView->toplevel->setFullscreenRequest(nullptr);
                }
                else
                {
                    if (toplevelView->toplevel->maximized())
                        toplevelView->toplevel->unsetMaximizedRequest();
                    else
                        toplevelView->toplevel->setMaximizedRequest();
                }
            }
        }
    }

    pressed = state == LPointer::Pressed;

    update();
}

void ToplevelButton::pointerLeaveEvent()
{
    if (!pressed)
        return;

    pressed = false;
    update();
}

void ToplevelButton::pointerMoveEvent(const LPoint &)
{
    if (!G::pointer()->resizingToplevel() && !G::pointer()->movingToplevel())
        cursor()->useDefault();
}
