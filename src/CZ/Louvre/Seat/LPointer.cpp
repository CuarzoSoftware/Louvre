#include <CZ/Louvre/Protocols/Wayland/RPointer.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Protocols/RelativePointer/RRelativePointer.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGestureSwipe.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGesturePinch.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGestureHold.h>
#include <CZ/Louvre/Protocols/PointerConstraints/RLockedPointer.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Private/LPointerPrivate.h>
#include <CZ/Louvre/Private/LToplevelRolePrivate.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Roles/LSubsurfaceRole.h>
#include <CZ/Louvre/Roles/LPopupRole.h>
#include <CZ/Louvre/Roles/LLayerRole.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Core/CZTime.h>
#include <cassert>

using namespace CZ;
using namespace CZ::Protocols;
using S = CZ::LPointer::LPointerPrivate::StateFlags;

LPointer::LPointer(const void *params) noexcept : LFactoryObject(FactoryObjectType), LPRIVATE_INIT_UNIQUE(LPointer)
{
    assert(params != nullptr && "Invalid parameter passed to LPointer constructor.");
    LPointer **ptr { (LPointer**) params };
    assert(*ptr == nullptr && *ptr == seat()->pointer() && "Only a single LPointer instance can exist.");
    *ptr = this;

    imp()->focus.setOnDestroyCallback([this](LSurface*) {
        focusChanged();
    });
}

LPointer::~LPointer() noexcept
{
    notifyDestruction();
}

void LPointer::setFocus(LSurface *surface) noexcept
{
    if (surface)
        setFocus(surface,
            SkIPoint::Make(
                cursor()->pos().x() - surface->rolePos().x(),
                cursor()->pos().y() - surface->rolePos().y()));
    else
        setFocus(nullptr, SkIPoint(0, 0));
}

void LPointer::setFocus(LSurface *surface, SkIPoint localPos) noexcept
{
    if (surface && grab() && surface != grab())
        return;

    CZWeak<LSurface> prevFocus { focus() };

    if (surface)
    {
        if (focus() == surface)
            return;

        if (imp()->state.has(S::PendingSwipeEndEvent))
        {
            CZPointerSwipeEndEvent e {};
            e.cancelled = true;
            sendSwipeEndEvent(e);
        }

        if (imp()->state.has(S::PendingPinchEndEvent))
        {
            CZPointerPinchEndEvent e {};
            e.cancelled = true;
            sendPinchEndEvent(e);
        }

        if (imp()->state.has(S::PendingHoldEndEvent))
        {
            CZPointerHoldEndEvent e {};
            e.cancelled = true;
            sendHoldEndEvent(e);
        }

        imp()->sendLeaveEvent(focus());

        CZPointerEnterEvent enterEvent;
        enterEvent.pos.set(localPos.x(), localPos.y());
        imp()->focus.reset();

        for (auto gSeat : surface->client()->seatGlobals())
        {
            for (auto rPointer : gSeat->pointerRes())
            {
                imp()->focus.reset(surface);
                rPointer->enter(enterEvent, surface->surfaceResource());
                rPointer->frame();
            }
        }
    }
    else
    {
        if (imp()->state.has(S::PendingSwipeEndEvent))
            sendSwipeEndEvent(CZPointerSwipeEndEvent());

        if (imp()->state.has(S::PendingPinchEndEvent))
            sendPinchEndEvent(CZPointerPinchEndEvent());

        if (imp()->state.has(S::PendingHoldEndEvent))
            sendHoldEndEvent(CZPointerHoldEndEvent());

        imp()->sendLeaveEvent(focus());
        imp()->focus.reset();
    }

    if (prevFocus.get() != focus())
        focusChanged();
}

void LPointer::sendMoveEvent(const CZPointerMoveEvent &event)
{
    if (!focus())
        return;

    Wayland::RPointer *lockedPointer { nullptr };

    if (focus()->imp()->current.lockedPointerRes && focus()->pointerConstraintEnabled() && focus()->pointerConstraintMode() == LSurface::Lock)
        lockedPointer = focus()->imp()->current.lockedPointerRes->pointerRes();

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerRes())
        {
            if (lockedPointer != rPointer)
                rPointer->motion(event);

            for (auto *rRelativePointer : rPointer->relativePointerRes())
                rRelativePointer->relativeMotion(event);

            rPointer->frame();
        }
    }
}

void LPointer::sendButtonEvent(const CZPointerButtonEvent &event)
{
    if (!focus())
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerRes())
        {
            rPointer->button(event);
            rPointer->frame();
        }
    }
}

void LPointer::sendScrollEvent(const CZPointerScrollEvent &event)
{
    if (!focus() || (!event.hasX && !event.hasY))
        return;

    const bool stopX { event.hasX && event.axes.x() == 0.f };
    const bool stopY { event.hasY && event.axes.y() == 0.f };
    const auto source { event.source == CZPointerScrollEvent::WheelLegacy ? CZPointerScrollEvent::Wheel : event.source };

    for (auto *gSeat : focus()->client()->seatGlobals())
    {
        // version < 8: 120 axes are not supported
        if (gSeat->version() < 8 && event.source == CZPointerScrollEvent::Wheel)
            continue;

        // version >= 8: Legacy events should not be sent
        if (gSeat->version() >= 8 && event.source == CZPointerScrollEvent::WheelLegacy)
            continue;

        // version < 6: Tilt event source does not exist
        if (gSeat->version() < 6 && event.source == CZPointerScrollEvent::WheelTilt)
            continue;

        for (auto *rPointer : gSeat->pointerRes())
        {
            // Since 5
            if (rPointer->axisSource(source))
            {
                if (rPointer->version() >= 9)
                {
                    if (event.hasX)
                        rPointer->axisRelativeDirection(WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.relativeDirectionX);

                    if (event.hasY)
                        rPointer->axisRelativeDirection(WL_POINTER_AXIS_VERTICAL_SCROLL, event.relativeDirectionY);
                }

                if (event.source == CZPointerScrollEvent::WheelTilt)
                {
                    if (event.hasX)
                        rPointer->axis(event.ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.axes.x());

                    if (event.hasY)
                        rPointer->axis(event.ms, WL_POINTER_AXIS_VERTICAL_SCROLL, event.axes.y());
                }
                else if (event.source == CZPointerScrollEvent::Wheel)
                {
                    if (event.hasX)
                    {
                        if (event.axesDiscrete.x() != 0)
                            rPointer->axisValue120(WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.axesDiscrete.x());

                        rPointer->axis(event.ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.axes.x());
                    }

                    if (event.hasY)
                    {
                        if (event.axesDiscrete.y() != 0)
                            rPointer->axisValue120(WL_POINTER_AXIS_VERTICAL_SCROLL, event.axesDiscrete.y());

                        rPointer->axis(event.ms, WL_POINTER_AXIS_VERTICAL_SCROLL, event.axes.y());
                    }
                }
                else if (event.source == CZPointerScrollEvent::WheelLegacy)
                {
                    if (event.hasX)
                    {
                        rPointer->axisDiscrete(WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.axesDiscrete.x());
                        rPointer->axis(event.ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.axes.x());
                    }

                    if (event.hasY)
                    {
                        rPointer->axisDiscrete(WL_POINTER_AXIS_VERTICAL_SCROLL, event.axesDiscrete.y());
                        rPointer->axis(event.ms, WL_POINTER_AXIS_VERTICAL_SCROLL, event.axes.y());
                    }
                }
                else if (event.source == CZPointerScrollEvent::Finger)
                {
                    if (stopX)
                        rPointer->axisStop(event.ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL);
                    else if (event.hasX)
                        rPointer->axis(event.ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.axes.x());

                    if (stopY)
                        rPointer->axisStop(event.ms, WL_POINTER_AXIS_VERTICAL_SCROLL);
                    else if (event.hasY)
                        rPointer->axis(event.ms, WL_POINTER_AXIS_VERTICAL_SCROLL, event.axes.y());
                }
                else // Continuous
                {
                    if (event.hasX)
                        rPointer->axis(event.ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.axes.x());

                    if (event.hasY)
                        rPointer->axis(event.ms, WL_POINTER_AXIS_VERTICAL_SCROLL, event.axes.y());
                }

                rPointer->frame();
            }
            // Since 1
            else
            {
                if (event.hasX)
                    rPointer->axis(event.ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.axes.x());

                if (event.hasY)
                    rPointer->axis(event.ms, WL_POINTER_AXIS_VERTICAL_SCROLL, event.axes.y());
            }
        }
    }
}

void LPointer::sendSwipeBeginEvent(const CZPointerSwipeBeginEvent &event)
{
    if (!focus() || imp()->state.has(S::PendingSwipeEndEvent))
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerRes())
        {
            for (auto rGestureSwipe : rPointer->gestureSwipeRes())
            {
                imp()->state.add(S::PendingSwipeEndEvent);
                rGestureSwipe->begin(event, focus()->surfaceResource());
            }
        }
    }
}

void LPointer::sendSwipeUpdateEvent(const CZPointerSwipeUpdateEvent &event)
{
    if (!focus() || !imp()->state.has(S::PendingSwipeEndEvent))
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerRes())
            for (auto rGestureSwipe :  rPointer->gestureSwipeRes())
                rGestureSwipe->update(event);
}

void LPointer::sendSwipeEndEvent(const CZPointerSwipeEndEvent &event)
{
    if (!focus() || !imp()->state.has(S::PendingSwipeEndEvent))
        return;

    imp()->state.remove(S::PendingSwipeEndEvent);

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerRes())
            for (auto rGestureSwipe :  rPointer->gestureSwipeRes())
                rGestureSwipe->end(event);
}

void LPointer::sendPinchBeginEvent(const CZPointerPinchBeginEvent &event)
{
    if (!focus() || imp()->state.has(S::PendingPinchEndEvent))
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerRes())
        {
            for (auto rGesturePinch : rPointer->gesturePinchRes())
            {
                imp()->state.add(S::PendingPinchEndEvent);
                rGesturePinch->begin(event, focus()->surfaceResource());
            }
        }
    }
}

void LPointer::sendPinchUpdateEvent(const CZPointerPinchUpdateEvent &event)
{
    if (!focus() || !imp()->state.has(S::PendingPinchEndEvent))
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerRes())
            for (auto rGesturePinch :  rPointer->gesturePinchRes())
                rGesturePinch->update(event);
}

void LPointer::sendPinchEndEvent(const CZPointerPinchEndEvent &event)
{
    if (!focus() || !imp()->state.has(S::PendingPinchEndEvent))
        return;

    imp()->state.remove(S::PendingPinchEndEvent);

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerRes())
            for (auto rGesturePinch :  rPointer->gesturePinchRes())
                rGesturePinch->end(event);
}

void LPointer::sendHoldBeginEvent(const CZPointerHoldBeginEvent &event)
{
    if (!focus() || imp()->state.has(S::PendingHoldEndEvent))
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerRes())
        {
            for (auto rGestureHold : rPointer->gestureHoldRes())
            {
                imp()->state.add(S::PendingHoldEndEvent);
                rGestureHold->begin(event, focus()->surfaceResource());
            }
        }
    }
}

void LPointer::sendHoldEndEvent(const CZPointerHoldEndEvent &event)
{
    if (!focus() || !imp()->state.has(S::PendingHoldEndEvent))
        return;

    imp()->state.remove(S::PendingHoldEndEvent);

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerRes())
            for (auto rGestureHold : rPointer->gestureHoldRes())
                rGestureHold->end(event);
}

void LPointer::setDraggingSurface(LSurface *surface) noexcept
{
    imp()->draggingSurface.reset(surface);
}

LSurface *LPointer::draggingSurface() const noexcept
{
    return imp()->draggingSurface;
}

LSurface *LPointer::focus() const noexcept
{
    return imp()->focus;
}

LSurface *LPointer::grab() const noexcept
{
    return imp()->grab;
}

void LPointer::setGrab(LSurface *surface) noexcept
{
    if (grab() == surface)
        return;

    if (focus() != surface)
        setFocus(nullptr);

    imp()->grab = surface;
}

void LPointer::LPointerPrivate::sendLeaveEvent(LSurface *surface) noexcept
{
    if (!surface)
        return;

    surface->enablePointerConstraint(false);

    CZPointerLeaveEvent leaveEvent;

    for (auto gSeat : surface->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerRes())
        {
            rPointer->leave(leaveEvent, surface->surfaceResource());
            rPointer->frame();
        }
    }
}
