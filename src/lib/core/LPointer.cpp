#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSeatPrivate.h>
#include <LCompositor.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LPopupRole.h>
#include <LTime.h>
#include <LKeyboard.h>
#include <LDNDManager.h>

using namespace Louvre;
using namespace Louvre::Protocols;

LPointer::LPointer(Params *params)
{
    L_UNUSED(params);
    m_imp = new LPointerPrivate();
    seat()->imp()->pointer = this;
}

LPointer::~LPointer()
{
    delete m_imp;
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
        if (focusSurface() == surface)
            return;

        imp()->sendLeaveEvent(focusSurface());

        Float24 x = wl_fixed_from_int(localPos.x());
        Float24 y = wl_fixed_from_int(localPos.y());
        imp()->pointerFocusSurface = nullptr;

        for (Wayland::GSeat *s : surface->client()->seatGlobals())
        {
            if (s->pointerResource())
            {
                UInt32 serial = LCompositor::nextSerial();
                imp()->pointerFocusSurface = surface;
                s->pointerResource()->imp()->serials.enter = serial;
                s->pointerResource()->enter(serial,
                                            surface->surfaceResource(),
                                            x,
                                            y);
                s->pointerResource()->frame();
            }
        }

        // Check if has a data device
        surface->client()->dataDevice().imp()->sendDNDEnterEventS(surface, x, y);
    }
    else
    {
        // Remove focus from focused surface
        imp()->sendLeaveEvent(focusSurface());
        imp()->pointerFocusSurface = nullptr;
    }

}

void LPointer::sendMoveEvent()
{
    if (focusSurface())
        sendMoveEvent(cursor()->pos() - focusSurface()->rolePos());
}

void LPointer::sendMoveEvent(const LPoint &localPos)
{
    if (!focusSurface())
        return;

    Float24 x = wl_fixed_from_int(localPos.x());
    Float24 y = wl_fixed_from_int(localPos.y());

    if (seat()->dndManager()->focus())
        seat()->dndManager()->focus()->client()->dataDevice().imp()->sendDNDMotionEventS(x, y);

    for (Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
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
    if (!focusSurface())
        return;

    for (Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
    {
        if (s->pointerResource())
        {
            UInt32 serial = LCompositor::nextSerial();
            UInt32 ms = LTime::ms();
            s->pointerResource()->imp()->serials.button = serial;
            s->pointerResource()->button(serial, ms, button, state);
            s->pointerResource()->frame();
        }
    }

    focusSurface()->client()->flush();
}

void LPointer::startResizingToplevel(LToplevelRole *toplevel, LToplevelRole::ResizeEdge edge, Int32 L, Int32 T, Int32 R, Int32 B)
{
    if (!toplevel)
        return;

    imp()->resizingToplevelConstraintBounds = LRect(L,T,R,B);
    imp()->resizingToplevel = toplevel;
    imp()->resizingToplevelEdge = edge;
    imp()->resizingToplevelInitWindowSize = toplevel->windowGeometry().size();
    imp()->resizingToplevelInitCursorPos = cursor()->pos();

    if (L != EdgeDisabled && toplevel->surface()->pos().x() < L)
        toplevel->surface()->setX(L);

    if (T != EdgeDisabled && toplevel->surface()->pos().y() < T)
        toplevel->surface()->setY(T);

    imp()->resizingToplevelInitPos = toplevel->surface()->pos();

    resizingToplevel()->configure(resizingToplevel()->states() | LToplevelRole::Resizing);
}

void LPointer::updateResizingToplevelSize()
{
    if (resizingToplevel())
    {
        LSize newSize = resizingToplevel()->calculateResizeSize(resizingToplevelInitCursorPos() - cursor()->pos(),
                                                                imp()->resizingToplevelInitWindowSize,
                                                                resizingToplevelEdge());
        // Con restricciones
        LToplevelRole::ResizeEdge edge =  resizingToplevelEdge();
        LPoint pos = resizingToplevel()->surface()->pos();
        LRect bounds = imp()->resizingToplevelConstraintBounds;
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


        resizingToplevel()->configure(newSize ,resizingToplevel()->states() | LToplevelRole::Resizing);
    }
}

void LPointer::updateResizingToplevelPos()
{
    if (resizingToplevel())
    {
        LSize s = resizingToplevelInitSize();
        LPoint p = resizingToplevelInitPos();
        LToplevelRole::ResizeEdge edge =  resizingToplevelEdge();

        if (edge ==  LToplevelRole::Top || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::TopRight)
            resizingToplevel()->surface()->setY(p.y() + (s.h() - resizingToplevel()->windowGeometry().h()));

        if (edge ==  LToplevelRole::Left || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::BottomLeft)
            resizingToplevel()->surface()->setX(p.x() + (s.w() - resizingToplevel()->windowGeometry().w()));
    }
}

void LPointer::stopResizingToplevel()
{
    if (resizingToplevel())
    {
        updateResizingToplevelSize();
        updateResizingToplevelPos();
        resizingToplevel()->configure(0, resizingToplevel()->states() &~ LToplevelRole::Resizing);
        imp()->resizingToplevel = nullptr;
    }
}

void LPointer::startMovingToplevel(LToplevelRole *toplevel, Int32 L, Int32 T, Int32 R, Int32 B)
{
    imp()->movingToplevelConstraintBounds = LRect(L,T,B,R);
    imp()->movingToplevelInitPos = toplevel->surface()->pos();
    imp()->movingToplevelInitCursorPos = cursor()->pos();
    imp()->movingToplevel = toplevel;
}

void LPointer::updateMovingToplevelPos()
{
    if (movingToplevel())
    {
        LPoint newPos = movingToplevelInitPos() - movingToplevelInitCursorPos() + cursor()->pos();

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

void LPointer::setDragginSurface(LSurface *surface)
{
    imp()->draggingSurface = surface;
}

void LPointer::dismissPopups()
{    
    list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
    {
        if ((*s)->popup())
            (*s)->popup()->sendPopupDoneEvent();
    }
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

const LPoint &LPointer::movingToplevelInitCursorPos() const
{
    return imp()->movingToplevelInitCursorPos;
}

const LPoint &LPointer::resizingToplevelInitPos() const
{
    return imp()->resizingToplevelInitPos;
}

const LPoint &LPointer::resizingToplevelInitCursorPos() const
{
    return imp()->resizingToplevelInitCursorPos;
}

const LSize &LPointer::resizingToplevelInitSize() const
{
    return imp()->resizingToplevelInitWindowSize;
}

LToplevelRole::ResizeEdge LPointer::resizingToplevelEdge() const
{
    return imp()->resizingToplevelEdge;
}

void LPointer::sendAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source)
{
    // If no surface has focus
    if (!focusSurface())
        return;

    Float24 aX = wl_fixed_from_double(axisX);
    Float24 aY = wl_fixed_from_double(axisY);
    Float24 dX = wl_fixed_from_int(discreteX);
    Float24 dY = wl_fixed_from_int(discreteY);

    UInt32 ms = LTime::ms();

    for (Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
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

                if (axisX == 0.0)
                    s->pointerResource()->axisStop(ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL);
                else
                    s->pointerResource()->axis(ms, WL_POINTER_AXIS_HORIZONTAL_SCROLL, aX);

                if (axisY == 0.0)
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
}

LSurface *LPointer::surfaceAt(const LPoint &point)
{
    for (list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin(); s != compositor()->surfaces().rend(); s++)
        if ((*s)->mapped() && !(*s)->minimized())
            if ((*s)->inputRegion().containsPoint(point - (*s)->rolePos()))
                return *s;

    return nullptr;
}

LSurface *LPointer::focusSurface() const
{
    return imp()->pointerFocusSurface;
}

void LPointer::LPointerPrivate::sendLeaveEvent(LSurface *surface)
{
    if (seat()->dndManager()->focus())
        seat()->dndManager()->focus()->client()->dataDevice().imp()->sendDNDLeaveEvent();

    if (!surface)
        return;

    for (Wayland::GSeat *s : surface->client()->seatGlobals())
    {
        if (s->pointerResource())
        {
            UInt32 serial = LCompositor::nextSerial();
            s->pointerResource()->imp()->serials.leave = serial;
            s->pointerResource()->leave(serial,
                                        surface->surfaceResource());

            s->pointerResource()->frame();
        }
    }
}
