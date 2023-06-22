#include "ToplevelRole.h"
#include "Output.h"
#include <LCursor.h>
#include <Compositor.h>
#include <Surface.h>
#include <LOutput.h>

ToplevelRole::ToplevelRole(Params *params) : LToplevelRole(params)
{

}

void ToplevelRole::configureRequest()
{
    // Get the main output
    LOutput *output = compositor()->cursor()->output();

    // Notifies the compositor's capabilities
    setWmCapabilities(WmCapabilities::WmWindowMenu | WmCapabilities::WmMinimize | WmCapabilities::WmMaximize | WmCapabilities::WmFullscreen);

    // Suggests to the Toplevel not to use a size larger than the output where the cursor is located
    configureBoundsC(output->sizeC() - LPoint(0, 32 * compositor()->globalScale()));

    // Adding the resize flag fixes Google Chrome window geometry bug
    configureC(0, Activated | Resizing);
}

void ToplevelRole::setMaximizedRequest()
{
    // Get the main output
    LOutput *output = compositor()->cursor()->output();

    // Tell the toplevel to maximize
    LSize size = output->sizeC()-LSize(0,32)*compositor()->globalScale();
    configureC(size, LToplevelRole::Activated | LToplevelRole::Maximized);
}

void ToplevelRole::setFullscreenRequest(LOutput *output)
{
    rectBeforeFullscreen = LRect(surface()->posC(), windowGeometryC().size());

    if (output)
        configureC(output->sizeC(), LToplevelRole::Activated | LToplevelRole::Fullscreen);
    else
    {
        // Get the main output
        LOutput *output = compositor()->cursor()->output();
        configureC(output->sizeC(), LToplevelRole::Activated | LToplevelRole::Fullscreen);
    }
}

void ToplevelRole::unsetFullscreenRequest()
{
    configureC(rectBeforeFullscreen.size(), LToplevelRole::Activated);
}

void ToplevelRole::maximizedChanged()
{
    // Get the main output
    LOutput *output = compositor()->cursor()->output();

    if (maximized())
    {
        compositor()->raiseSurface(surface());
        surface()->setPosC(output->posC()+LPoint(0,32)*compositor()->globalScale());
        surface()->setMinimized(false);
    }
}

void ToplevelRole::fullscreenChanged()
{
    Output *output = (Output*)compositor()->cursor()->output();

    if (fullscreen())
    {
        surface()->setPosC(output->posC());
        output->fullscreenSurface = surface();
        compositor()->raiseSurface(surface());
    }
    else
    {
        surface()->setPosC(rectBeforeFullscreen.pos());
        if (output->fullscreenSurface == surface())
           output->fullscreenSurface = nullptr;
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
