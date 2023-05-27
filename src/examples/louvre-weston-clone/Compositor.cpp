#include <Compositor.h>
#include <LSeat.h>
#include <LKeyboard.h>
#include <LOutputManager.h>
#include <LOutput.h>
#include <LOutputMode.h>

#include <Output.h>
#include <Surface.h>
#include <ToplevelRole.h>
#include <Popup.h>
#include <Pointer.h>

Compositor::Compositor():LCompositor(){}


LOutput *Compositor::createOutputRequest()
{
    return new Output();
}

LSurface *Compositor::createSurfaceRequest(LSurface::Params *params)
{
    Surface *newSurface = new Surface(params);
    newSurface->repaint();
    return newSurface;
}

void Compositor::destroySurfaceRequest(LSurface *s)
{
    if (s == fullscreenSurface)
        fullscreenSurface = nullptr;
}

LToplevelRole *Compositor::createToplevelRoleRequest(LToplevelRole::Params *params)
{
    return new ToplevelRole(params);
}

LPopupRole *Compositor::createPopupRoleRequest(LPopupRole::Params *params)
{
    return new Popup(params);
}

LPointer *Compositor::createPointerRequest(LPointer::Params *params)
{
    return new LPointer(params);
}


