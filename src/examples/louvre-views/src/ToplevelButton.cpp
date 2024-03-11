#include "Toplevel.h"
#include "ToplevelButton.h"
#include "ToplevelView.h"
#include "Global.h"
#include "InputRect.h"
#include "Pointer.h"
#include <LKeyboard.h>
#include <LSeat.h>
#include <LCursor.h>
#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>

ToplevelButton::ToplevelButton(LView *parent, ToplevelView *toplevelView, ButtonType type) : UITextureView(G::ButtonDisabled, parent)
{
    this->toplevelView = toplevelView;
    this->buttonType = type;
    setBufferScale(2);
    enablePointerEvents(true);
    update();
}

void ToplevelButton::update()
{
    if (buttonType == Close)
    {
        if (toplevelView->buttonsContainer.pointerIsOver())
        {
            if (pressed)
                setTextureIndex(G::CloseButtonEnabledPressed);
            else
                setTextureIndex(G::CloseButtonEnabledHover);
        }
        else
        {
            if (toplevelView->toplevel->activated())
                setTextureIndex(G::CloseButtonEnabled);
            else
                setTextureIndex(G::ButtonDisabled);
        }
    }
    else if (buttonType == Minimize)
    {
        if (toplevelView->toplevel->fullscreen())
        {
            setTextureIndex(G::ButtonDisabled);
        }
        else
        {
            if (toplevelView->buttonsContainer.pointerIsOver())
            {
                if (pressed)
                    setTextureIndex(G::MinimizeButtonEnabledPressed);
                else
                    setTextureIndex(G::MinimizeButtonEnabledHover);
            }
            else
            {
                if (toplevelView->toplevel->activated())
                    setTextureIndex(G::MinimizeButtonEnabled);
                else
                    setTextureIndex(G::ButtonDisabled);
            }
        }
    }
    else
    {

        bool altMode = !seat()->keyboard()->isKeyCodePressed(KEY_LEFTALT) || toplevelView->toplevel->fullscreen();

        if (toplevelView->buttonsContainer.pointerIsOver())
        {
            if (pressed)
            {
                if (altMode)
                {
                    if (toplevelView->toplevel->fullscreen())
                        setTextureIndex(G::UnfullscreenButtonEnabledPressed);
                    else
                        setTextureIndex(G::FullscreenButtonEnabledPressed);
                }
                else
                    setTextureIndex(G::MaximizeButtonEnabledPressed);
            }
            else
            {
                if (altMode)
                {
                    if (toplevelView->toplevel->fullscreen())
                        setTextureIndex(G::UnfullscreenButtonEnabledHover);
                    else
                        setTextureIndex(G::FullscreenButtonEnabledHover);
                }
                else
                    setTextureIndex(G::MaximizeButtonEnabledHover);
            }
        }
        else
        {
            if (toplevelView->toplevel->activated())
                setTextureIndex(G::MaximizeButtonEnabled);
            else
                setTextureIndex(G::ButtonDisabled);
        }
    }
}

void ToplevelButton::pointerButtonEvent(const LPointerButtonEvent &event)
{
    if (event.button() != LPointerButtonEvent::Left)
        return;

    if (pressed && event.state() == LPointerButtonEvent::Released)
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

    pressed = event.state() == LPointerButtonEvent::Pressed;
    update();
}

void ToplevelButton::pointerLeaveEvent(const LPointerLeaveEvent &)
{
    if (!pressed)
        return;

    pressed = false;
    update();
}

void ToplevelButton::pointerMoveEvent(const LPointerMoveEvent &)
{
    if (!toplevelView->toplevel->resizeSession().isActive() && !toplevelView->toplevel->moveSession().isActive())
        cursor()->useDefault();
}
