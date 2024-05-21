#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
#include <LTouchDownEvent.h>
#include <LTouchPoint.h>
#include <LCursor.h>
#include <Compositor.h>
#include <Surface.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LTouch.h>

#include "ToplevelRole.h"
#include "Output.h"

ToplevelRole::ToplevelRole(const void *params) noexcept : LToplevelRole(params) {}

void ToplevelRole::configurationChanged(LBitset<ConfigurationChanges> changes)
{
    if (!changes.check(StateChanged))
        return;

    const LBitset<State> stateChanges { current().state ^ previous().state };
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
                configureState(pending().state & ~Maximized);
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
                configureState(pending().state & ~Fullscreen);
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
}

void ToplevelRole::configureRequest() noexcept
{
    configureSize(0, 0);
    configureState(Activated | pending().state);
    configureDecorationMode(ClientSide);
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

    statesBeforeFullscreen = current().state;
    rectBeforeFullscreen = LRect(surface()->pos(), windowGeometry().size());

    configureSize(output->size());
    configureState(LToplevelRole::Activated | LToplevelRole::Fullscreen);
}

void ToplevelRole::unsetFullscreenRequest() noexcept
{
    configureSize(rectBeforeFullscreen.size());
    configureState(statesBeforeFullscreen);
}
