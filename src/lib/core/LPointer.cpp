#include <protocols/Wayland/RPointer.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/RelativePointer/RRelativePointer.h>
#include <protocols/PointerGestures/RGestureSwipe.h>
#include <protocols/PointerGestures/RGesturePinch.h>
#include <protocols/PointerGestures/RGestureHold.h>
#include <protocols/PointerConstraints/RLockedPointer.h>
#include <private/LClientPrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LSurfacePrivate.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LPopupRole.h>
#include <LTime.h>
#include <LKeyboard.h>
#include <LDND.h>
#include <cassert>

using namespace Louvre;
using namespace Louvre::Protocols;
using S = Louvre::LPointer::LPointerPrivate::StateFlags;

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

LPointer::~LPointer()
{
    notifyDestruction();
}

void LPointer::setFocus(LSurface *surface) noexcept
{
    if (surface)
        setFocus(surface, cursor()->pos() - surface->rolePos());
    else
        setFocus(nullptr, 0);
}

void LPointer::setFocus(LSurface *surface, const LPoint &localPos) noexcept
{
    LWeak<LSurface> prevFocus { focus() };

    if (surface)
    {
        if (focus() == surface)
            return;

        if (imp()->state.check(S::PendingSwipeEndEvent))
            sendSwipeEndEvent(LPointerSwipeEndEvent(0, true));

        if (imp()->state.check(S::PendingPinchEndEvent))
            sendPinchEndEvent(LPointerPinchEndEvent(0, true));

        if (imp()->state.check(S::PendingHoldEndEvent))
            sendHoldEndEvent(LPointerHoldEndEvent(0, true));

        imp()->sendLeaveEvent(focus());

        LPointerEnterEvent enterEvent;
        enterEvent.localPos = localPos;
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
        if (imp()->state.check(S::PendingSwipeEndEvent))
            sendSwipeEndEvent(LPointerSwipeEndEvent());

        if (imp()->state.check(S::PendingPinchEndEvent))
            sendPinchEndEvent(LPointerPinchEndEvent());

        if (imp()->state.check(S::PendingHoldEndEvent))
            sendHoldEndEvent(LPointerHoldEndEvent());

        imp()->sendLeaveEvent(focus());
        imp()->focus.reset();
    }

    if (prevFocus.get() != focus())
        focusChanged();
}

void LPointer::sendMoveEvent(const LPointerMoveEvent &event)
{
    if (!focus())
        return;

    Wayland::RPointer *lockedPointer { nullptr };

    if (focus()->pointerConstraintEnabled() && focus()->pointerConstraintMode() == LSurface::Lock)
        lockedPointer = focus()->imp()->lockedPointerRes->pointerRes();

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerRes())
        {
            if (lockedPointer != rPointer)
                rPointer->motion(event);

            for (auto rRelativePointer : rPointer->relativePointerRes())
                rRelativePointer->relativeMotion(event);

            rPointer->frame();
        }
    }
}

void LPointer::sendButtonEvent(const LPointerButtonEvent &event)
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

void LPointer::sendScrollEvent(const LPointerScrollEvent &event)
{
    if (!focus())
        return;

    const UInt32 naturalX { naturalScrollingXEnabled() ? WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED : WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL };
    const UInt32 naturalY { naturalScrollingYEnabled() ? WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED : WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL };
    const bool stopX { event.axes().x() == 0.f && imp()->axisXprev != 0.f };
    const bool stopY { event.axes().y() == 0.f && imp()->axisYprev != 0.f };

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerRes())
        {
            // Since 5
            if (rPointer->axisSource(event.source()))
            {
                Float32 aX, aY;
                Int32 dX, dY;

                if (rPointer->axisRelativeDirection(WL_POINTER_AXIS_HORIZONTAL_SCROLL, naturalX))
                {
                    rPointer->axisRelativeDirection(WL_POINTER_AXIS_VERTICAL_SCROLL, naturalY);
                    aX = event.axes().x();
                    aY = event.axes().y();
                    dX = event.axes120().x();
                    dY = event.axes120().y();
                }
                // Less than v9
                else
                {
                    if (naturalScrollingXEnabled())
                    {
                        aX = -event.axes().x();
                        dX = -event.axes120().x();
                    }
                    else
                    {
                        aX = event.axes().x();
                        dX = event.axes120().x();
                    }

                    if (naturalScrollingYEnabled())
                    {
                        aY = -event.axes().y();
                        dY = -event.axes120().y();
                    }
                    else
                    {
                        aY = event.axes().y();
                        dY = event.axes120().y();
                    }
                }

                if (event.source() == LPointerScrollEvent::Wheel)
                {
                    if (rPointer->version() < 8)
                    {
                        rPointer->axis(event.ms(), WL_POINTER_AXIS_HORIZONTAL_SCROLL, aX);
                        rPointer->axisDiscrete(WL_POINTER_AXIS_HORIZONTAL_SCROLL, aX);

                        rPointer->axis(event.ms(), WL_POINTER_AXIS_VERTICAL_SCROLL, aY);
                        rPointer->axisDiscrete(WL_POINTER_AXIS_VERTICAL_SCROLL, aY);
                    }
                    else
                    {
                        if (dX != 0)
                        {
                            rPointer->axisValue120(WL_POINTER_AXIS_HORIZONTAL_SCROLL, dX);
                            rPointer->axis(event.ms(), WL_POINTER_AXIS_HORIZONTAL_SCROLL, aX);
                        }

                        if (dY != 0)
                        {
                            rPointer->axisValue120(WL_POINTER_AXIS_VERTICAL_SCROLL, dY);
                            rPointer->axis(event.ms(), WL_POINTER_AXIS_VERTICAL_SCROLL, aY);
                        }
                    }
                }
                else
                {
                    if (stopX)
                        rPointer->axisStop(event.ms(), WL_POINTER_AXIS_HORIZONTAL_SCROLL);
                    else
                        rPointer->axis(event.ms(), WL_POINTER_AXIS_HORIZONTAL_SCROLL, aX);

                    if (stopY)
                        rPointer->axisStop(event.ms(), WL_POINTER_AXIS_VERTICAL_SCROLL);
                    else
                        rPointer->axis(event.ms(), WL_POINTER_AXIS_VERTICAL_SCROLL, aY);
                }

                rPointer->frame();
            }
            // Since 1
            else
            {
                rPointer->axis(event.ms(), WL_POINTER_AXIS_HORIZONTAL_SCROLL, event.axes().x());
                rPointer->axis(event.ms(), WL_POINTER_AXIS_VERTICAL_SCROLL, event.axes().y());
            }
        }
    }

    imp()->axisXprev = event.axes().x();
    imp()->axisYprev = event.axes().y();
}

void LPointer::sendSwipeBeginEvent(const LPointerSwipeBeginEvent &event)
{
    if (!focus() || imp()->state.check(S::PendingSwipeEndEvent))
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

void LPointer::sendSwipeUpdateEvent(const LPointerSwipeUpdateEvent &event)
{
    if (!focus() || !imp()->state.check(S::PendingSwipeEndEvent))
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerRes())
            for (auto rGestureSwipe :  rPointer->gestureSwipeRes())
                rGestureSwipe->update(event);
}

void LPointer::sendSwipeEndEvent(const LPointerSwipeEndEvent &event)
{
    if (!focus() || !imp()->state.check(S::PendingSwipeEndEvent))
        return;

    imp()->state.remove(S::PendingSwipeEndEvent);

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerRes())
            for (auto rGestureSwipe :  rPointer->gestureSwipeRes())
                rGestureSwipe->end(event);
}

void LPointer::sendPinchBeginEvent(const LPointerPinchBeginEvent &event)
{
    if (!focus() || imp()->state.check(S::PendingPinchEndEvent))
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

void LPointer::sendPinchUpdateEvent(const LPointerPinchUpdateEvent &event)
{
    if (!focus() || !imp()->state.check(S::PendingPinchEndEvent))
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerRes())
            for (auto rGesturePinch :  rPointer->gesturePinchRes())
                rGesturePinch->update(event);
}

void LPointer::sendPinchEndEvent(const LPointerPinchEndEvent &event)
{
    if (!focus() || !imp()->state.check(S::PendingPinchEndEvent))
        return;

    imp()->state.remove(S::PendingPinchEndEvent);

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerRes())
            for (auto rGesturePinch :  rPointer->gesturePinchRes())
                rGesturePinch->end(event);
}

void LPointer::sendHoldBeginEvent(const LPointerHoldBeginEvent &event)
{
    if (!focus() || imp()->state.check(S::PendingHoldEndEvent))
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

void LPointer::sendHoldEndEvent(const LPointerHoldEndEvent &event)
{
    if (!focus() || !imp()->state.check(S::PendingHoldEndEvent))
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

void LPointer::enableNaturalScrollingX(bool enabled) noexcept
{
    imp()->state.setFlag(LPointerPrivate::NaturalScrollX, enabled);
}

void LPointer::enableNaturalScrollingY(bool enabled) noexcept
{
    imp()->state.setFlag(LPointerPrivate::NaturalScrollY, enabled);
}

bool LPointer::naturalScrollingXEnabled() const noexcept
{
    return imp()->state.check(LPointerPrivate::NaturalScrollX);
}

bool LPointer::naturalScrollingYEnabled() const noexcept
{
    return imp()->state.check(LPointerPrivate::NaturalScrollY);
}

const std::vector<LPointerButtonEvent::Button> &LPointer::pressedButtons() const noexcept
{
    return imp()->pressedButtons;
}

bool LPointer::isButtonPressed(LPointerButtonEvent::Button button) const noexcept
{
    for (auto btn : imp()->pressedButtons)
        if (btn == button)
            return true;
    return false;
}

LSurface *LPointer::surfaceAt(const LPoint &point)
{
    retry:
    compositor()->imp()->surfacesListChanged = false;

    for (std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin(); s != compositor()->surfaces().rend(); s++)
        if ((*s)->mapped() && !(*s)->minimized())
        {
            if ((*s)->inputRegion().containsPoint(point - (*s)->rolePos()))
                return *s;

            if (compositor()->imp()->surfacesListChanged)
                goto retry;
        }

    return nullptr;
}

LSurface *LPointer::focus() const noexcept
{
    return imp()->focus;
}

void LPointer::LPointerPrivate::sendLeaveEvent(LSurface *surface) noexcept
{
    if (!surface)
        return;

    surface->enablePointerConstraint(false);

    LPointerLeaveEvent leaveEvent;

    for (auto gSeat : surface->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerRes())
        {
            rPointer->leave(leaveEvent, surface->surfaceResource());
            rPointer->frame();
        }
    }
}
