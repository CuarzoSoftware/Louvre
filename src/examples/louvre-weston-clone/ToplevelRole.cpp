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
                surface()->setPos(output->pos() + LPoint(0, 32));
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

    const LSize size { output->size() - LSize(0, 32) };
    configureSize(size);
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

void ToplevelRole::startMoveRequest(const LEvent &triggeringEvent) noexcept
{
    if (fullscreen())
        return;

    if (triggeringEvent.type() == LEvent::Type::Touch)
    {
        if (triggeringEvent.subtype() != LEvent::Subtype::Down)
           return;

        if (!cursor()->output())
           return;

        const LTouchDownEvent& touchDownEvent { static_cast<const LTouchDownEvent&>(triggeringEvent) };
        LTouchPoint *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id()) };

        if (!touchPoint)
           return;

        if (touchPoint->surface() != surface())
           return;

        const LPoint initDragPoint { LTouch::toGlobal(cursor()->output(), touchPoint->pos()) };

        moveSession().start(triggeringEvent, initDragPoint, EdgeDisabled, 32);
    }
    else if (surface()->hasPointerFocus())
        moveSession().start(triggeringEvent, cursor()->pos(), EdgeDisabled, 32);
}

void ToplevelRole::startResizeRequest(const LEvent &triggeringEvent, ResizeEdge edge) noexcept
{
    if (fullscreen())
        return;

    if (triggeringEvent.type() == LEvent::Type::Touch)
    {
        if (triggeringEvent.subtype() != LEvent::Subtype::Down)
           return;

        if (!cursor()->output())
           return;

        const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(triggeringEvent) };
        LTouchPoint *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id()) };

        if (!touchPoint)
           return;

        if (touchPoint->surface() != surface())
           return;

        const LPoint initDragPoint { LTouch::toGlobal(cursor()->output(), touchPoint->pos()) };

        resizeSession().start(triggeringEvent, edge, initDragPoint, LSize(128, 128), EdgeDisabled, 32);
    }
    else if (surface()->hasPointerFocus())
        resizeSession().start(triggeringEvent, edge, cursor()->pos(), LSize(128, 128), EdgeDisabled, 32);
}
