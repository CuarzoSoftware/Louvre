#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/GSeat.h>
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
    return imp()->lastCursorRequestWasHide;
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

        imp()->sendLeaveEvent(focus());

        Float24 x = wl_fixed_from_int(localPos.x());
        Float24 y = wl_fixed_from_int(localPos.y());
        imp()->pointerFocusSurface = nullptr;

        for (Wayland::GSeat *s : surface->client()->seatGlobals())
        {
            if (s->pointerResource())
            {
                UInt32 serial = LTime::nextSerial();
                imp()->pointerFocusSurface = surface;
                s->pointerResource()->imp()->serials.enter = serial;
                s->pointerResource()->enter(serial,
                                            surface->surfaceResource(),
                                            x,
                                            y);
                s->pointerResource()->frame();
            }
        }

        surface->client()->dataDevice().imp()->sendDNDEnterEventS(surface, x, y);
    }
    else
    {
        // Remove focus from focused surface
        imp()->sendLeaveEvent(focus());
        imp()->pointerFocusSurface = nullptr;
    }

}

void LPointer::sendMoveEvent()
{
    if (focus())
        sendMoveEvent(cursor()->pos() - focus()->rolePos());
}

void LPointer::sendMoveEvent(const LPoint &localPos)
{
    if (!focus())
        return;

    Float24 x = wl_fixed_from_int(localPos.x());
    Float24 y = wl_fixed_from_int(localPos.y());

    if (seat()->dndManager()->focus())
        seat()->dndManager()->focus()->client()->dataDevice().imp()->sendDNDMotionEventS(x, y);

    for (Wayland::GSeat *s : focus()->client()->seatGlobals())
    {
        if (s->pointerResource())
        {
            UInt32 ms = LTime::ms();
            s->pointerResource()->motion(ms, x, y);
            s->pointerResource()->frame();
        }
    }   
}

void LPointer::sendButtonEvent(Button button, ButtonState state)
{
    if (!focus())
        return;

    for (Wayland::GSeat *s : focus()->client()->seatGlobals())
    {
        if (s->pointerResource())
        {
            UInt32 serial = LTime::nextSerial();
            UInt32 ms = LTime::ms();
            s->pointerResource()->imp()->serials.button = serial;
            s->pointerResource()->button(serial, ms, button, state);
            s->pointerResource()->frame();
        }
    }

    focus()->client()->flush();
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

void LPointer::sendAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, AxisSource source)
{
    // If no surface has focus
    if (!focus())
        return;

    Float24 aX = wl_fixed_from_double(axisX);
    Float24 aY = wl_fixed_from_double(axisY);
    Float24 dX = wl_fixed_from_int(discreteX);
    Float24 dY = wl_fixed_from_int(discreteY);

    UInt32 ms = LTime::ms();

    for (Wayland::GSeat *s : focus()->client()->seatGlobals())
    {
        if (s->pointerResource())
        {
            // Since 5
            if (s->pointerResource()->axisSource(source))
            {
                s->pointerResource()->axisRelativeDirection(WL_POINTER_AXIS_HORIZONTAL_SCROLL, 0 /* 0 = IDENTICAL */);
                s->pointerResource()->axisRelativeDirection(WL_POINTER_AXIS_VERTICAL_SCROLL, 0 /* 0 = IDENTICAL */);

                if (source == LPointer::AxisSource::Wheel)
                {
                    if (!s->pointerResource()->axisValue120(WL_POINTER_AXIS_HORIZONTAL_SCROLL, dX))
                    {
                        s->pointerResource()->axisDiscrete(WL_POINTER_AXIS_HORIZONTAL_SCROLL, aX);
                        s->pointerResource()->axisDiscrete(WL_POINTER_AXIS_VERTICAL_SCROLL, aY);
                    }
                    else
                        s->pointerResource()->axisValue120(WL_POINTER_AXIS_VERTICAL_SCROLL, dY);
                }

                if (axisX == 0.0 && imp()->axisXprev != 0.0)
                    s->pointerResource()->axisStop(ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL);
                else
                    s->pointerResource()->axis(ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL, aX);

                if (axisY == 0.0 && imp()->axisYprev != 0.0)
                   s->pointerResource()->axisStop(ms, WL_POINTER_AXIS_VERTICAL_SCROLL);
                else
                    s->pointerResource()->axis(ms, WL_POINTER_AXIS_VERTICAL_SCROLL, aY);

                s->pointerResource()->frame();
            }
            // Since 1
            else
            {
                s->pointerResource()->axis(
                    ms,
                    aX,
                    aY);
            }
        }
    }

    imp()->axisXprev = axisX;
    imp()->axisYprev = axisY;
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
    if (seat()->dndManager()->focus() && seat()->dndManager()->focus() == surface)
        seat()->dndManager()->focus()->client()->dataDevice().imp()->sendDNDLeaveEvent();

    if (!surface)
        return;

    for (Wayland::GSeat *s : surface->client()->seatGlobals())
    {
        if (s->pointerResource())
        {
            UInt32 serial = LTime::nextSerial();
            s->pointerResource()->imp()->serials.leave = serial;
            s->pointerResource()->leave(serial,
                                        surface->surfaceResource());

            s->pointerResource()->frame();
        }
    }
}
