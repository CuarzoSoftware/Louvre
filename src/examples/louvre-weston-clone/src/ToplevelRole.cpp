#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
#include <LTouchDownEvent.h>
#include <LTouchPoint.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LTouch.h>

#include "Compositor.h"
#include "ToplevelRole.h"
#include "Output.h"
#include "Surface.h"

void ToplevelRole::atomsChanged(LBitset<AtomChanges> changes, const Atoms &prev)
{
    if (!changes.check(StateChanged))
        return;

    const LBitset<State> stateChanges { state() ^ prev.state };
    Output *output { static_cast<Output*>(cursor()->output()) };

    if (stateChanges.check(Maximized))
    {
        if (maximized())
        {
            if (output)
            {
                surface()->raise();
                surface()->setPos(output->pos() + output->availableGeometry().pos());
                surface()->setMinimized(false);
            }
            else
            {
                configureSize(0, 0);
                configureState(pendingConfiguration().state & ~Maximized);
            }
        }
    }

    if (stateChanges.check(Fullscreen))
    {
        if (fullscreen())
        {
            if (output)
            {
                surface()->setPos(output->pos());
                output->fullscreenSurface = surface();
                surface()->raise();
            }
            else
            {
                configureSize(0, 0);
                configureState(pendingConfiguration().state & ~Fullscreen);
            }
        }
        else if (output)
        {
            surface()->setPos(rectBeforeFullscreen.pos());
            if (output->fullscreenSurface == surface())
                output->fullscreenSurface = nullptr;

            output->fullDamage();
        }
    }

    if (stateChanges.check(Activated) && activated())
    {
        surface()->setMinimized(false);
        surface()->raise();
    }
}

void ToplevelRole::configureRequest() noexcept
{
    configureSize(0, 0);
    configureState(Activated | pendingConfiguration().state);
    configureDecorationMode(ClientSide);
    configureCapabilities(MaximizeCap | MinimizeCap | FullscreenCap);

    if (cursor()->output())
        configureBounds(cursor()->output()->availableGeometry().size());
}

void ToplevelRole::setMaximizedRequest() noexcept
{
    const LOutput *output { cursor()->output() };

    if (!output)
        return;

    configureSize(output->availableGeometry().size());
    configureState(LToplevelRole::Activated | LToplevelRole::Maximized);
}

void ToplevelRole::setFullscreenRequest(LOutput *preferredOutput) noexcept
{
    const LOutput *output { preferredOutput != nullptr ? preferredOutput : cursor()->output()};

    if (!output)
        output = cursor()->output();

    statesBeforeFullscreen = state();
    rectBeforeFullscreen = LRect(surface()->pos(), windowGeometry().size());

    configureSize(output->size());
    configureState(LToplevelRole::Activated | LToplevelRole::Fullscreen);
}

void ToplevelRole::unsetFullscreenRequest() noexcept
{
    configureSize(rectBeforeFullscreen.size());
    configureState(statesBeforeFullscreen);
}
