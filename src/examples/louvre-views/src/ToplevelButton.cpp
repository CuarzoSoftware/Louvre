#include "Toplevel.h"
#include "ToplevelButton.h"
#include "ToplevelView.h"
#include "Global.h"
#include "InputRect.h"
#include "Pointer.h"
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
                setTexture(G::toplevelTextures().activeCloseButtonPressed);
            else
                setTexture(G::toplevelTextures().activeCloseButtonHover);
        }
        else
        {
            if (toplevelView->toplevel->activated())
                setTexture(G::toplevelTextures().activeCloseButton);
            else
                setTexture(G::toplevelTextures().inactiveButton);
        }
    }
    else if (buttonType == Minimize)
    {
        if (toplevelView->toplevel->fullscreen())
        {
            setTexture(G::toplevelTextures().inactiveButton);
        }
        else
        {
            if (toplevelView->buttonsContainer->pointerIsOver())
            {
                if (pressed)
                    setTexture(G::toplevelTextures().activeMinimizeButtonPressed);
                else
                    setTexture(G::toplevelTextures().activeMinimizeButtonHover);
            }
            else
            {
                if (toplevelView->toplevel->activated())
                    setTexture(G::toplevelTextures().activeMinimizeButton);
                else
                    setTexture(G::toplevelTextures().inactiveButton);
            }
        }
    }
    else
    {

        bool altMode = seat()->keyboard()->isKeyCodePressed(KEY_LEFTALT) || toplevelView->toplevel->fullscreen();

        if (toplevelView->buttonsContainer->pointerIsOver())
        {
            if (pressed)
            {
                if (altMode)
                {
                    if (toplevelView->toplevel->fullscreen())
                        setTexture(G::toplevelTextures().activeUnfullscreenButtonPressed);
                    else
                        setTexture(G::toplevelTextures().activeFullscreenButtonPressed);
                }
                else
                    setTexture(G::toplevelTextures().activeMaximizeButtonPressed);
            }
            else
            {
                if (altMode)
                {
                    if (toplevelView->toplevel->fullscreen())
                        setTexture(G::toplevelTextures().activeUnfullscreenButtonHover);
                    else
                        setTexture(G::toplevelTextures().activeFullscreenButtonHover);
                }
                else
                    setTexture(G::toplevelTextures().activeMaximizeButtonHover);
            }
        }
        else
        {
            if (toplevelView->toplevel->activated())
                setTexture(G::toplevelTextures().activeMaximizeButton);
            else
                setTexture(G::toplevelTextures().inactiveButton);
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

                if (altMode)
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
