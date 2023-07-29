#include "ToplevelTopbarButton.h"
#include "ToplevelTopbar.h"
#include "Toplevel.h"
#include "Global.h"

ToplevelTopbarButton::ToplevelTopbarButton(Toplevel *toplevel, TopBarButtonType type)
{
    this->toplevel = toplevel;
    this->type = type;
    enableInput(true);
    setParent(toplevel->topBar);
    setSize(LSize(TOPLEVEL_TOPBAR_BUTTON_SIZE));

    LPoint pos;
    pos.setY( (TOPLEVEL_TOPBAR_HEIGHT - TOPLEVEL_TOPBAR_BUTTON_SIZE) / 2);

    if (type == CloseButton)
    {
        setColor(1.f, 0.f, 0.f);
        pos.setX(pos.y());
    }
    else if (type == MinimizeButton)
    {
        setColor(1.f, 0.8f, 0.1f);
        pos.setX(pos.y() * 2 + TOPLEVEL_TOPBAR_BUTTON_SIZE);
    }
    else
    {
        setColor(0.f, 1.f, 0.f);
        pos.setX(pos.y() * 3 + TOPLEVEL_TOPBAR_BUTTON_SIZE * 2);
    }

    setPos(pos);
}

void ToplevelTopbarButton::pointerEnterEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);
    setOpacity(0.5f);
}

void ToplevelTopbarButton::pointerLeaveEvent()
{
    setOpacity(1.f);
}

void ToplevelTopbarButton::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    if (button == LPointer::Left && state == LPointer::Released)
    {
        if (type == CloseButton)
            toplevel->close();
        else if (type == MinimizeButton)
            toplevel->setMinimizedRequest();
        else
        {
            if (toplevel->maximized())
                toplevel->unsetMaximizedRequest();
            else
                toplevel->setMaximizedRequest();
        }
    }
}
