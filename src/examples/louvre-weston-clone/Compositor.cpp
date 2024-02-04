#include "LTime.h"
#include "Seat.h"
#include <Compositor.h>
#include <LSeat.h>
#include <LKeyboard.h>
#include <LOutput.h>
#include <LOutputMode.h>

#include <Output.h>
#include <Surface.h>
#include <ToplevelRole.h>
#include <Popup.h>
#include <Pointer.h>

Compositor::Compositor():LCompositor(){}

LSeat *Compositor::createSeatRequest(void *params)
{
    return new Seat(params);
}

LOutput *Compositor::createOutputRequest()
{
    return new Output();
}

LSurface *Compositor::createSurfaceRequest(void *params)
{
    Surface *newSurface = new Surface(params);
    return newSurface;
}

void Compositor::destroySurfaceRequest(LSurface *s)
{
    for (Output *output : (std::vector<Output*>&)outputs())
        if (s == output->fullscreenSurface)
            output->fullscreenSurface = nullptr;
}

void Compositor::cursorInitialized()
{
    pointerCursor = LXCursor::loadXCursorB("hand2");
}

LToplevelRole *Compositor::createToplevelRoleRequest(void *params)
{
    return new ToplevelRole(params);
}

LPopupRole *Compositor::createPopupRoleRequest(void *params)
{
    return new Popup(params);
}

LPointer *Compositor::createPointerRequest(void *params)
{
    return new Pointer(params);
}
