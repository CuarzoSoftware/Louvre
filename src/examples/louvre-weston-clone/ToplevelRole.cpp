#include "ToplevelRole.h"
#include <LCursor.h>
#include <Compositor.h>
#include <Surface.h>
#include <LOutput.h>

ToplevelRole::ToplevelRole(Params *params) : LToplevelRole(params)
{

}

void ToplevelRole::setMaximizedRequest()
{
    // Get the main output
    LOutput *output = compositor()->cursor()->output();

    // Tell the toplevel to maximize
    LSize size = output->sizeC()-LSize(0,32)*compositor()->globalScale();
    configureC(size, LToplevelRole::Activated);
    configureC(size, LToplevelRole::Activated | LToplevelRole::Maximized);

}

void ToplevelRole::maximizedChanged()
{
    // Get the main output
    LOutput *output = compositor()->cursor()->output();

    if(maximized())
    {
        compositor()->raiseSurface(surface());
        surface()->setPosC(output->posC()+LPoint(0,32)*compositor()->globalScale());
        surface()->setMinimized(false);
        ((Surface*)(surface()))->repaint();
    }

}

void ToplevelRole::fullscreenChanged()
{
    Compositor *comp = (Compositor*)compositor();

    LOutput *output = compositor()->cursor()->output();

    if(fullscreen())
    {
        surface()->setPosC(output->posC());
        comp->fullscreenSurface = surface();
        compositor()->raiseSurface(surface());
    }
    else
    {
        if(comp->fullscreenSurface == surface())
        {
            comp->fullscreenSurface = nullptr;
        }
    }
}

void ToplevelRole::startMoveRequest()
{
    seat()->pointer()->startMovingToplevelC(this, LPointer::EdgeDisabled, 32 * compositor()->globalScale());
}

void ToplevelRole::startResizeRequest(ResizeEdge edge)
{
    seat()->pointer()->startResizingToplevelC(this, edge, LPointer::EdgeDisabled, 32 * compositor()->globalScale());
}
