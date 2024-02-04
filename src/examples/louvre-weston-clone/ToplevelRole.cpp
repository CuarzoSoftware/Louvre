#include "ToplevelRole.h"
#include "Output.h"
#include <LCursor.h>
#include <Compositor.h>
#include <Surface.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>

ToplevelRole::ToplevelRole(void *params) : LToplevelRole(params) {}

void ToplevelRole::configureRequest()
{
    setDecorationMode(ClientSide);
    configure(0, Activated | pendingStates());
}

void ToplevelRole::setMaximizedRequest()
{
    LOutput *output = cursor()->output();

    // Tell the toplevel to maximize
    LSize size = output->size() - LSize(0, 32);
    configure(size, LToplevelRole::Activated | LToplevelRole::Maximized);
}

void ToplevelRole::setFullscreenRequest(LOutput *output)
{
    statesBeforeFullscreen = states();
    rectBeforeFullscreen = LRect(surface()->pos(), windowGeometry().size());

    if (!output)
        output = cursor()->output();

    configure(output->size(), LToplevelRole::Activated | LToplevelRole::Fullscreen);
}

void ToplevelRole::unsetFullscreenRequest()
{
    configure(rectBeforeFullscreen.size(), statesBeforeFullscreen);
}

void ToplevelRole::maximizedChanged()
{
    // Get the main output
    LOutput *output = compositor()->cursor()->output();

    if (maximized())
    {
        surface()->raise();
        surface()->setPos(output->pos() + LPoint(0, 32));
        surface()->setMinimized(false);
    }
}

void ToplevelRole::fullscreenChanged()
{
    Output *output = (Output*)compositor()->cursor()->output();

    if (fullscreen())
    {
        surface()->setPos(output->pos());
        output->fullscreenSurface = surface();
        surface()->raise();
    }
    else
    {
        surface()->setPos(rectBeforeFullscreen.pos());
        if (output->fullscreenSurface == surface())
           output->fullscreenSurface = nullptr;
    }
}

void ToplevelRole::startMoveRequest()
{
    if (!fullscreen())
        seat()->pointer()->startMovingToplevel(this, cursor()->pos(), LPointer::EdgeDisabled, 32);
}

void ToplevelRole::startResizeRequest(ResizeEdge edge)
{
    if (!fullscreen())
        seat()->pointer()->startResizingToplevel(this,
                                                 edge,
                                                 cursor()->pos(),
                                                 LSize(128, 128),
                                                 LPointer::EdgeDisabled, 32);
}
