#include "ToplevelTopbar.h"
#include "ToplevelTopbarButton.h"
#include "Surface.h"
#include "Toplevel.h"
#include "Pointer.h"

ToplevelTopbar::ToplevelTopbar(Toplevel *toplevel) : LSolidColorView(1.f, 1.f, 1.f, 1.f, ((class Surface*)toplevel->surface())->view)
{
    this->toplevel = toplevel;
    toplevel->topBar = this;
    update();
    enableInput(true);

    closeButton = new ToplevelTopbarButton(toplevel, ToplevelTopbarButton::CloseButton);
    minimizeButton = new ToplevelTopbarButton(toplevel, ToplevelTopbarButton::MinimizeButton);
    maximizeButton = new ToplevelTopbarButton(toplevel, ToplevelTopbarButton::MaximizeButton);
}

ToplevelTopbar::~ToplevelTopbar()
{
    delete closeButton;
    delete minimizeButton;
    delete maximizeButton;
}

void ToplevelTopbar::update()
{
    setPos(LPoint(0, -TOPLEVEL_TOPBAR_HEIGHT));
    setSize(LSize(toplevel->surface()->size().w(), TOPLEVEL_TOPBAR_HEIGHT));
}

void ToplevelTopbar::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    if (button != LPointer::Left)
        return;

    if (state == LPointer::Pressed)
    {
        seat()->pointer()->setFocus(toplevel->surface());
        toplevel->startMoveRequest();
    }
}
