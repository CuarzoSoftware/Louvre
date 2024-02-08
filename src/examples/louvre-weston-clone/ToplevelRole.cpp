#include "ToplevelRole.h"
#include "Output.h"
#include <LCursor.h>
#include <Compositor.h>
#include <Surface.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>

ToplevelRole::ToplevelRole(const void *params) : LToplevelRole(params) {}

void ToplevelRole::configureRequest()
{
    setDecorationMode(ClientSide);
    configure(0, Activated | pendingStates());
}

void ToplevelRole::setMaximizedRequest()
{
    const LOutput *output { cursor()->output() };

    if (!output)
        return;

    const LSize size { output->size() - LSize(0, 32) };
    configure(size, LToplevelRole::Activated | LToplevelRole::Maximized);
}

void ToplevelRole::setFullscreenRequest(LOutput *preferredOutput)
{
    const LOutput *output { preferredOutput != nullptr ? preferredOutput : cursor()->output()};

    if (!output)
        output = cursor()->output();

    statesBeforeFullscreen = states();
    rectBeforeFullscreen = LRect(surface()->pos(), windowGeometry().size());

    configure(output->size(), LToplevelRole::Activated | LToplevelRole::Fullscreen);
}

void ToplevelRole::unsetFullscreenRequest()
{
    configure(rectBeforeFullscreen.size(), statesBeforeFullscreen);
}

void ToplevelRole::maximizedChanged()
{
    const LOutput *output { cursor()->output() };

    if (maximized())
    {
        if (output)
        {
            surface()->raise();
            surface()->setPos(output->pos() + LPoint(0, 32));
            surface()->setMinimized(false);
        }
        else
            configure(LSize(0, 0), pendingStates() & ~Maximized);
    }
}

void ToplevelRole::fullscreenChanged()
{
    Output *output { (Output*)cursor()->output() };

    if (fullscreen())
    {
        if (output)
        {
            surface()->setPos(output->pos());
            output->fullscreenSurface = surface();
            surface()->raise();
        }
        else
            configure(LSize(0, 0), pendingStates() & ~Fullscreen);
    }
    else if (output)
    {
        surface()->setPos(rectBeforeFullscreen.pos());
        if (output->fullscreenSurface == surface())
           output->fullscreenSurface = nullptr;
    }

    if (output)
        output->fullDamage();
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
