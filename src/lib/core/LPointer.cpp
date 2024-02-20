#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/RelativePointer/RRelativePointer.h>
#include <protocols/PointerGestures/RGestureSwipe.h>
#include <protocols/PointerGestures/RGesturePinch.h>
#include <protocols/PointerGestures/RGestureHold.h>
#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LPopupRole.h>
#include <LTime.h>
#include <LKeyboard.h>
#include <LDNDManager.h>

using namespace Louvre;
using namespace Louvre::Protocols;
using S = Louvre::LPointer::LPointerPrivate::StateFlags;

LPointer::LPointer(const void *params) : LPRIVATE_INIT_UNIQUE(LPointer)
{
    L_UNUSED(params);
    seat()->imp()->pointer = this;
}

LPointer::~LPointer() {}

LCursorRole *LPointer::lastCursorRequest() const
{
    return imp()->lastCursorRequest;
}

bool LPointer::lastCursorRequestWasHide() const
{
    return imp()->state.check(S::LastCursorRequestWasHide);
}

void LPointer::setFocus(LSurface *surface)
{
    if (surface)
        setFocus(surface, cursor()->pos() - surface->rolePos());
    else
        setFocus(nullptr, 0);
}

void LPointer::setFocus(LSurface *surface, const LPoint &localPos)
{
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
        imp()->pointerFocusSurface = nullptr;

        for (auto gSeat : surface->client()->seatGlobals())
        {
            for (auto rPointer : gSeat->pointerResources())
            {
                imp()->pointerFocusSurface = surface;
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
        imp()->pointerFocusSurface = nullptr;
    }
}

void LPointer::sendMoveEvent(const LPointerMoveEvent &event)
{
    if (!focus())
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerResources())
        {
            rPointer->motion(event);
            rPointer->frame();

            for (auto rRelativePointer : rPointer->relativePointerResources())
                rRelativePointer->relativeMotion(event);
        }
    }
}

void LPointer::sendButtonEvent(const LPointerButtonEvent &event)
{
    if (!focus())
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerResources())
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
        for (auto rPointer : gSeat->pointerResources())
        {
            // Since 5
            if (rPointer->axisSource(event.source()))
            {
                Float24 aX, aY, dX ,dY;

                if (rPointer->axisRelativeDirection(WL_POINTER_AXIS_HORIZONTAL_SCROLL, naturalX))
                {
                    rPointer->axisRelativeDirection(WL_POINTER_AXIS_VERTICAL_SCROLL, naturalY);
                    aX = wl_fixed_from_double(event.axes().x());
                    aY = wl_fixed_from_double(event.axes().y());
                    dX = wl_fixed_from_double(event.axes120().x());
                    dY = wl_fixed_from_double(event.axes120().y());
                }
                // Less than v9
                else
                {
                    if (naturalScrollingXEnabled())
                    {
                        aX = wl_fixed_from_double(-event.axes().x());
                        dX = wl_fixed_from_double(-event.axes120().x());
                    }
                    else
                    {
                        aX = wl_fixed_from_double(event.axes().x());
                        dX = wl_fixed_from_double(event.axes120().x());
                    }

                    if (naturalScrollingYEnabled())
                    {
                        aY = wl_fixed_from_double(-event.axes().y());
                        dY = wl_fixed_from_double(-event.axes120().y());
                    }
                    else
                    {
                        aY = wl_fixed_from_double(event.axes().y());
                        dY = wl_fixed_from_double(event.axes120().y());
                    }
                }

                if (event.source() == LPointerScrollEvent::Wheel)
                {
                    if (!rPointer->axisValue120(WL_POINTER_AXIS_HORIZONTAL_SCROLL, dX))
                    {
                        rPointer->axisDiscrete(WL_POINTER_AXIS_HORIZONTAL_SCROLL, aX);
                        rPointer->axisDiscrete(WL_POINTER_AXIS_VERTICAL_SCROLL, aY);
                    }
                    else
                        rPointer->axisValue120(WL_POINTER_AXIS_VERTICAL_SCROLL, dY);
                }

                if (stopX)
                    rPointer->axisStop(event.ms(), WL_POINTER_AXIS_HORIZONTAL_SCROLL);
                else
                    rPointer->axis(event.ms(), WL_POINTER_AXIS_HORIZONTAL_SCROLL, aX);

                if (stopY)
                    rPointer->axisStop(event.ms(), WL_POINTER_AXIS_VERTICAL_SCROLL);
                else
                    rPointer->axis(event.ms(), WL_POINTER_AXIS_VERTICAL_SCROLL, aY);

                rPointer->frame();
            }
            // Since 1
            else
                rPointer->axis(event.ms(),
                               wl_fixed_from_double(event.axes().x()),
                               wl_fixed_from_double(event.axes().y()));
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
        for (auto rPointer : gSeat->pointerResources())
        {
            for (auto rGestureSwipe : rPointer->gestureSwipeResources())
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
        for (auto rPointer : gSeat->pointerResources())
            for (auto rGestureSwipe :  rPointer->gestureSwipeResources())
                rGestureSwipe->update(event);
}

void LPointer::sendSwipeEndEvent(const LPointerSwipeEndEvent &event)
{
    if (!focus() || !imp()->state.check(S::PendingSwipeEndEvent))
        return;

    imp()->state.remove(S::PendingSwipeEndEvent);

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerResources())
            for (auto rGestureSwipe :  rPointer->gestureSwipeResources())
                rGestureSwipe->end(event);
}

void LPointer::sendPinchBeginEvent(const LPointerPinchBeginEvent &event)
{
    if (!focus() || imp()->state.check(S::PendingPinchEndEvent))
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerResources())
        {
            for (auto rGesturePinch : rPointer->gesturePinchResources())
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
        for (auto rPointer : gSeat->pointerResources())
            for (auto rGesturePinch :  rPointer->gesturePinchResources())
                rGesturePinch->update(event);
}

void LPointer::sendPinchEndEvent(const LPointerPinchEndEvent &event)
{
    if (!focus() || !imp()->state.check(S::PendingPinchEndEvent))
        return;

    imp()->state.remove(S::PendingPinchEndEvent);

    for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rPointer : gSeat->pointerResources())
            for (auto rGesturePinch :  rPointer->gesturePinchResources())
                rGesturePinch->end(event);
}

void LPointer::sendHoldBeginEvent(const LPointerHoldBeginEvent &event)
{
    if (!focus() || imp()->state.check(S::PendingHoldEndEvent))
        return;

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerResources())
        {
            for (auto rGestureHold : rPointer->gestureHoldResources())
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
        for (auto rPointer : gSeat->pointerResources())
            for (auto rGestureHold : rPointer->gestureHoldResources())
                rGestureHold->end(event);
}

void LPointer::startResizingToplevel(LToplevelRole *toplevel,
                                     LToplevelRole::ResizeEdge edge,
                                     const LPoint &pointerPos,
                                     const LSize &minSize,
                                     Int32 L, Int32 T, Int32 R, Int32 B)
{
    if (!toplevel)
        return;

    imp()->resizingToplevel = toplevel;

    toplevel->imp()->resizingMinSize = minSize;
    toplevel->imp()->resizingConstraintBounds = LRect(L,T,R,B);
    toplevel->imp()->resizingEdge = edge;
    toplevel->imp()->resizingInitWindowSize = toplevel->windowGeometry().size();
    toplevel->imp()->resizingInitPointerPos = pointerPos;
    toplevel->imp()->resizingCurrentPointerPos = pointerPos;

    if (L != EdgeDisabled && toplevel->surface()->pos().x() < L)
        toplevel->surface()->setX(L);

    if (T != EdgeDisabled && toplevel->surface()->pos().y() < T)
        toplevel->surface()->setY(T);

    toplevel->imp()->resizingInitPos = toplevel->surface()->pos();

    resizingToplevel()->configure(resizingToplevel()->size(), LToplevelRole::Activated | LToplevelRole::Resizing);
}

void LPointer::updateResizingToplevelSize(const LPoint &pointerPos)
{
    if (resizingToplevel())
    {
        resizingToplevel()->imp()->resizingCurrentPointerPos = pointerPos;
        LSize newSize = resizingToplevel()->calculateResizeSize(resizingToplevel()->imp()->resizingInitPointerPos - pointerPos,
                                                                resizingToplevel()->imp()->resizingInitWindowSize,
                                                                resizingToplevel()->imp()->resizingEdge);
        // Con restricciones
        LToplevelRole::ResizeEdge edge =  resizingToplevel()->imp()->resizingEdge;
        LPoint pos = resizingToplevel()->surface()->pos();
        LRect bounds = resizingToplevel()->imp()->resizingConstraintBounds;
        LSize size = resizingToplevel()->windowGeometry().size();

        // Top
        if (bounds.y() != EdgeDisabled && (edge ==  LToplevelRole::Top || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::TopRight))
        {
            if (pos.y() - (newSize.y() - size.y()) < bounds.y())
                newSize.setH(pos.y() + size.h() - bounds.y());
        }
        // Bottom
        else if (bounds.h() != EdgeDisabled && (edge ==  LToplevelRole::Bottom || edge ==  LToplevelRole::BottomLeft || edge ==  LToplevelRole::BottomRight))
        {
            if (pos.y() + newSize.h() > bounds.h())
                newSize.setH(bounds.h() - pos.y());
        }

        // Left
        if ( bounds.x() != EdgeDisabled && (edge ==  LToplevelRole::Left || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::BottomLeft))
        {
            if (pos.x() - (newSize.x() - size.x()) < bounds.x())
                newSize.setW(pos.x() + size.w() - bounds.x());
        }
        // Right
        else if ( bounds.w() != EdgeDisabled && (edge ==  LToplevelRole::Right || edge ==  LToplevelRole::TopRight || edge ==  LToplevelRole::BottomRight))
        {
            if (pos.x() + newSize.w() > bounds.w())
                newSize.setW(bounds.w() - pos.x());
        }

        if (newSize.w() < resizingToplevel()->imp()->resizingMinSize.w())
            newSize.setW(resizingToplevel()->imp()->resizingMinSize.w());

        if (newSize.h() < resizingToplevel()->imp()->resizingMinSize.h())
            newSize.setH(resizingToplevel()->imp()->resizingMinSize.h());

        resizingToplevel()->configure(newSize, LToplevelRole::Activated | LToplevelRole::Resizing);
    }
}

void LPointer::updateResizingToplevelPos()
{
    if (resizingToplevel())
        resizingToplevel()->updateResizingPos();
}

void LPointer::stopResizingToplevel()
{
    if(resizingToplevel())
    {
        updateResizingToplevelSize(cursor()->pos());
        updateResizingToplevelPos();
        resizingToplevel()->configure(0, resizingToplevel()->pendingStates() &~ LToplevelRole::Resizing);
        imp()->resizingToplevel = nullptr;
    }
}

void LPointer::startMovingToplevel(LToplevelRole *toplevel, const LPoint &pointerPos, Int32 L, Int32 T, Int32 R, Int32 B)
{
    imp()->movingToplevelConstraintBounds = LRect(L,T,B,R);
    imp()->movingToplevelInitPos = toplevel->surface()->pos();
    imp()->movingToplevelInitPointerPos = pointerPos;
    imp()->movingToplevel = toplevel;
}

void LPointer::updateMovingToplevelPos(const LPoint &pointerPos)
{
    if (movingToplevel())
    {
        LPoint newPos = movingToplevelInitPos() - movingToplevelInitPointerPos() + pointerPos;

        if (imp()->movingToplevelConstraintBounds.w() != EdgeDisabled && newPos.x() > imp()->movingToplevelConstraintBounds.w())
            newPos.setX(imp()->movingToplevelConstraintBounds.w());

        if (imp()->movingToplevelConstraintBounds.x() != EdgeDisabled && newPos.x() < imp()->movingToplevelConstraintBounds.x())
            newPos.setX(imp()->movingToplevelConstraintBounds.x());

        if (imp()->movingToplevelConstraintBounds.h() != EdgeDisabled && newPos.y() > imp()->movingToplevelConstraintBounds.h())
            newPos.setY(imp()->movingToplevelConstraintBounds.h());

        if (imp()->movingToplevelConstraintBounds.y() != EdgeDisabled && newPos.y() < imp()->movingToplevelConstraintBounds.y())
            newPos.setY(imp()->movingToplevelConstraintBounds.y());

        movingToplevel()->surface()->setPos(newPos);
    }
}

void LPointer::stopMovingToplevel()
{
    imp()->movingToplevel = nullptr;
}

void LPointer::setDraggingSurface(LSurface *surface)
{
    imp()->draggingSurface = surface;
}

void LPointer::dismissPopups()
{
    std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
    {
        if ((*s)->popup())
            (*s)->popup()->dismiss();
    }
}

void LPointer::enableNaturalScrollingX(bool enabled)
{
    imp()->state.setFlag(LPointerPrivate::NaturalScrollX, enabled);
}

void LPointer::enableNaturalScrollingY(bool enabled)
{
    imp()->state.setFlag(LPointerPrivate::NaturalScrollY, enabled);
}

bool LPointer::naturalScrollingXEnabled() const
{
    return imp()->state.check(LPointerPrivate::NaturalScrollX);
}

bool LPointer::naturalScrollingYEnabled() const
{
    return imp()->state.check(LPointerPrivate::NaturalScrollY);
}

const std::vector<LPointer::Button> &LPointer::pressedKeys() const
{
    return imp()->pressedButtons;
}

bool LPointer::isButtonPressed(Button button) const
{
    for (Button btn : imp()->pressedButtons)
    {
        if (btn == button)
            return true;
    }
    return false;
}

LSurface *LPointer::draggingSurface() const
{
    return imp()->draggingSurface;
}

LToplevelRole *LPointer::resizingToplevel() const
{
    return imp()->resizingToplevel;
}

LToplevelRole *LPointer::movingToplevel() const
{
    return imp()->movingToplevel;
}

const LPoint &LPointer::movingToplevelInitPos() const
{
    return imp()->movingToplevelInitPos;
}

const LPoint &LPointer::movingToplevelInitPointerPos() const
{
    return imp()->movingToplevelInitPointerPos;
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

LSurface *LPointer::focus() const
{
    return imp()->pointerFocusSurface;
}

void LPointer::LPointerPrivate::sendLeaveEvent(LSurface *surface)
{
    if (!surface)
        return;

    LPointerLeaveEvent leaveEvent;

    for (auto gSeat : surface->client()->seatGlobals())
    {
        for (auto rPointer : gSeat->pointerResources())
        {
            rPointer->leave(leaveEvent, surface->surfaceResource());
            rPointer->frame();
        }
    }
}
