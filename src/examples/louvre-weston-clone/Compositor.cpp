#include <Compositor.h>
#include <LSeat.h>
#include <LKeyboard.h>
#include <LOutput.h>
#include <LOutputMode.h>

#include "Output.h"
#include "Surface.h"
#include "ToplevelRole.h"
#include "Pointer.h"
#include "Seat.h"
#include "Popup.h"


Compositor::Compositor():LCompositor(){}

void Compositor::cursorInitialized()
{
    pointerCursor = LXCursor::loadXCursorB("hand2");
}

LSeat           *Compositor::createSeatRequest(const void *params)         { return new Seat(params); }
LOutput         *Compositor::createOutputRequest(const void *params)       { return new Output(params); }
LSurface        *Compositor::createSurfaceRequest(const void *params)      { return new Surface(params); }
LToplevelRole   *Compositor::createToplevelRoleRequest(const void *params) { return new ToplevelRole(params); }
LPopupRole      *Compositor::createPopupRoleRequest(const void *params)    { return new Popup(params); }
LPointer        *Compositor::createPointerRequest(const void *params)      { return new Pointer(params); }

void Compositor::destroySurfaceRequest(LSurface *s)
{
    for (Output *output : (const std::vector<Output*>&)outputs())
        if (s == output->fullscreenSurface)
            output->fullscreenSurface = nullptr;
}

