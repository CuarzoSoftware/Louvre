#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LToplevelRolePrivate.h>

#include <LSeat.h>
#include <LCompositor.h>
#include <LWayland.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LPopupRole.h>
#include <LTime.h>
#include <LKeyboard.h>
#include <LDNDManager.h>

using namespace Louvre;

LPointer::LPointer(Params *params)
{
    m_imp = new LPointerPrivate();
    imp()->seat = params->seat;
}

LPointer::~LPointer()
{
    delete m_imp;
}

LSeat *LPointer::seat() const
{
    return imp()->seat;
}

LCompositor *LPointer::compositor() const
{
    return seat()->compositor();
}

LCursor *LPointer::cursor() const
{
    return compositor()->cursor();
}

void LPointer::setFocusC(LSurface *surface)
{
    if(surface)
        setFocusC(surface, cursor()->posC() - surface->rolePosC());
    else
        setFocusC(nullptr, LPoint());
}

void LPointer::setFocusC(LSurface *surface, const LPoint &localPos)
{
    // If surface is not nullptr
    if(surface)
    {

        // If already has focus
        if(focusSurface() == surface)
            return;

        // Remove focus from focused surface
        imp()->sendLeaveEvent(focusSurface());

        // Set focus
        if(surface->client()->pointerResource())
        {
            // Send focus event
            imp()->sendEnterEvent(surface, localPos);
            imp()->pointerFocusSurface = surface;
        }
        else
            imp()->pointerFocusSurface = nullptr;

        // Check if has a data device
        if(surface->client()->dataDevice())
            surface->client()->dataDevice()->imp()->sendDNDEnterEvent(surface,localPos.x(),localPos.y());

    }

    // If surface is nullptr
    else
    {
        // Remove focus from focused surface
        imp()->sendLeaveEvent(focusSurface());
        imp()->pointerFocusSurface = nullptr;
    }
}

void LPointer::sendMoveEventC()
{
    if(focusSurface())
        sendMoveEventC(cursor()->posC() - focusSurface()->rolePosC());
}

void LPointer::sendMoveEventC(const LPoint &localPos)
{
    if(seat()->dndManager()->focus() && seat()->dndManager()->focus()->client()->dataDevice())
        seat()->dndManager()->focus()->client()->dataDevice()->imp()->sendDNDMotionEvent(localPos.x(),localPos.y());

    // If no surface has focus surface
    if(!focusSurface())
        return;

    // If do not have a wl_pointer
    if(!focusSurface()->client()->pointerResource())
        return;

    // Send pointer move event to the focused surface
    wl_pointer_send_motion(focusSurface()->client()->pointerResource(),
                           LTime::ms(),
                           wl_fixed_from_int(localPos.x()/compositor()->globalScale()),
                           wl_fixed_from_int(localPos.y()/compositor()->globalScale()));

#if LOUVRE_SEAT_VERSION >=5
    if(wl_resource_get_version(focusSurface()->client()->pointerResource()) >= 5)
        wl_pointer_send_frame(focusSurface()->client()->pointerResource());
#endif
}

void LPointer::sendButtonEvent(Button button, ButtonState state)
{
    // If no surface has focus
    if(!focusSurface())
        return;

    // If do not have a wl_pointer
    if(!focusSurface()->client()->pointerResource())
        return;

    // Send pointer button event
    imp()->lastPointerButtonEventSerial = LWayland::nextSerial();
    focusSurface()->client()->imp()->serials.lastPointerButtonEventSerial = imp()->lastPointerButtonEventSerial;
    wl_pointer_send_button(focusSurface()->client()->pointerResource(),
                           focusSurface()->client()->imp()->serials.lastPointerButtonEventSerial,
                           LTime::ms(),
                           button,
                           state);

#if LOUVRE_SEAT_VERSION >=5
    if(wl_resource_get_version(focusSurface()->client()->pointerResource()) >= 5)
        wl_pointer_send_frame(focusSurface()->client()->pointerResource());
#endif
}


void LPointer::startResizingToplevelC(LToplevelRole *toplevel, LToplevelRole::ResizeEdge edge, Int32 L, Int32 T, Int32 R, Int32 B)
{
    if(!toplevel)
        return;

    imp()->resizingToplevelConstraintBounds = LRect(L,T,R,B);
    imp()->resizingToplevel = toplevel;
    imp()->resizingToplevelEdge = edge;
    imp()->resizingToplevelInitWindowSize = toplevel->windowGeometryC().bottomRight();
    imp()->resizingToplevelInitCursorPos = cursor()->posC();

    if(L != EdgeDisabled && toplevel->surface()->posC().x() < L)
        toplevel->surface()->setXC(L);

    if(T != EdgeDisabled && toplevel->surface()->posC().y() < T)
        toplevel->surface()->setYC(T);

    imp()->resizingToplevelInitPos = toplevel->surface()->posC();

    resizingToplevel()->configureC(resizingToplevel()->states() | LToplevelRole::Resizing);
}

void LPointer::updateResizingToplevelSize()
{
    if(resizingToplevel())
    {
        LSize newSize = resizingToplevel()->calculateResizeSize(resizingToplevelInitCursorPos()-cursor()->posC(),
                                                                imp()->resizingToplevelInitWindowSize,
                                                                resizingToplevelEdge());
        // Con restricciones
        LToplevelRole::ResizeEdge edge =  resizingToplevelEdge();
        LPoint pos = resizingToplevel()->surface()->posC();
        LRect bounds = imp()->resizingToplevelConstraintBounds;
        LSize size = resizingToplevel()->windowGeometryC().bottomRight();

        // Top
        if(bounds.y() != EdgeDisabled && (edge ==  LToplevelRole::Top || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::TopRight))
        {
            if(pos.y() - (newSize.y() - size.y()) < bounds.y())
                newSize.setH(pos.y() + size.h() - bounds.y());
        }
        // Bottom
        else if(bounds.h() != EdgeDisabled && (edge ==  LToplevelRole::Bottom || edge ==  LToplevelRole::BottomLeft || edge ==  LToplevelRole::BottomRight))
        {
            if(pos.y() + newSize.h() > bounds.h())
                newSize.setH(bounds.h() - pos.y());
        }


        // Left
        if( bounds.x() != EdgeDisabled && (edge ==  LToplevelRole::Left || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::BottomLeft))
        {
            if(pos.x() - (newSize.x() - size.x()) < bounds.x())
                newSize.setW(pos.x() + size.w() - bounds.x());
        }
        // Right
        else if( bounds.w() != EdgeDisabled && (edge ==  LToplevelRole::Right || edge ==  LToplevelRole::TopRight || edge ==  LToplevelRole::BottomRight))
        {
            if(pos.x() + newSize.w() > bounds.w())
                newSize.setW(bounds.w() - pos.x());
        }


        resizingToplevel()->configureC(newSize ,resizingToplevel()->states() | LToplevelRole::Resizing);
    }
}

void LPointer::updateResizingToplevelPos()
{
    if(resizingToplevel())
    {
        LSize s = resizingToplevelInitSize();
        LPoint p = resizingToplevelInitPos();
        LToplevelRole::ResizeEdge edge =  resizingToplevelEdge();

        if(edge ==  LToplevelRole::Top || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::TopRight)
            resizingToplevel()->surface()->setYC(p.y() + (s.h() - resizingToplevel()->windowGeometryC().h()));

        if(edge ==  LToplevelRole::Left || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::BottomLeft)
            resizingToplevel()->surface()->setXC(p.x() + (s.w() - resizingToplevel()->windowGeometryC().w()));
    }
}

void LPointer::stopResizingToplevel()
{
    if(resizingToplevel())
    {
        updateResizingToplevelSize();
        updateResizingToplevelPos();
        resizingToplevel()->configureC(0, resizingToplevel()->states() &~ LToplevelRole::Resizing);
        imp()->resizingToplevel = nullptr;
    }
}

void LPointer::startMovingToplevelC(LToplevelRole *toplevel, Int32 L, Int32 T, Int32 R, Int32 B)
{
    imp()->movingToplevelConstraintBounds = LRect(L,T,B,R);
    imp()->movingToplevelInitPos = toplevel->surface()->posC();
    imp()->movingToplevelInitCursorPos = cursor()->posC();
    imp()->movingToplevel = toplevel;
}

void LPointer::updateMovingToplevelPos()
{
    if(movingToplevel())
    {
        LPoint newPos = movingToplevelInitPos() - movingToplevelInitCursorPos() + cursor()->posC();

        if(imp()->movingToplevelConstraintBounds.w() != EdgeDisabled && newPos.x() > imp()->movingToplevelConstraintBounds.w())
            newPos.setX(imp()->movingToplevelConstraintBounds.w());

        if(imp()->movingToplevelConstraintBounds.x() != EdgeDisabled && newPos.x() < imp()->movingToplevelConstraintBounds.x())
            newPos.setX(imp()->movingToplevelConstraintBounds.x());

        if(imp()->movingToplevelConstraintBounds.h() != EdgeDisabled && newPos.y() > imp()->movingToplevelConstraintBounds.h())
            newPos.setY(imp()->movingToplevelConstraintBounds.h());

        if(imp()->movingToplevelConstraintBounds.y() != EdgeDisabled && newPos.y() < imp()->movingToplevelConstraintBounds.y())
            newPos.setY(imp()->movingToplevelConstraintBounds.y());

        movingToplevel()->surface()->setPosC(newPos);
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
        if((*s)->popup())
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

UInt32 LPointer::lastPointerEnterEventSerial() const
{
    return imp()->lastPointerEnterEventSerial;
}

UInt32 LPointer::lastPointerLeaveEventSerial() const
{
    return imp()->lastPointerLeaveEventSerial;
}

UInt32 LPointer::lastPointerButtonEventSerial() const
{
    return imp()->lastPointerButtonEventSerial;
}

#if LOUVRE_SEAT_VERSION >= 5

void LPointer::sendAxisEvent(double x, double y, UInt32 source)
{
    // If no surface has focus
    if(!focusSurface())
        return;

    // If do not have a wl_pointer
    wl_resource *res = focusSurface()->client()->pointerResource();
    if(!res)
        return;

    UInt32 time = LTime::ms();

    if(wl_resource_get_version(res) >= 5)
    {
        if(source == WL_POINTER_AXIS_SOURCE_WHEEL || source == WL_POINTER_AXIS_SOURCE_WHEEL_TILT)
        {
            wl_pointer_send_axis_discrete(res,WL_POINTER_AXIS_HORIZONTAL_SCROLL,scrollWheelStep().x());
            wl_pointer_send_axis(res,time,WL_POINTER_AXIS_HORIZONTAL_SCROLL,wl_fixed_from_double(x));
            wl_pointer_send_axis_discrete(res,WL_POINTER_AXIS_VERTICAL_SCROLL,scrollWheelStep().y());
            wl_pointer_send_axis(res,time,WL_POINTER_AXIS_VERTICAL_SCROLL,wl_fixed_from_double(y));
        }
        else
        {
            if(x == 0.0)
                wl_pointer_send_axis_stop(res,time,WL_POINTER_AXIS_HORIZONTAL_SCROLL);
            else
                wl_pointer_send_axis(res,time,WL_POINTER_AXIS_HORIZONTAL_SCROLL,wl_fixed_from_double(x));

            if(y == 0.0)
                wl_pointer_send_axis_stop(res,time,WL_POINTER_AXIS_VERTICAL_SCROLL);
            else
                wl_pointer_send_axis(res,time,WL_POINTER_AXIS_VERTICAL_SCROLL,wl_fixed_from_double(y));
        }
        wl_pointer_send_axis_source(res,source);
        wl_pointer_send_frame(res);
    }
    else
    {
        wl_pointer_send_axis(res,time,WL_POINTER_AXIS_HORIZONTAL_SCROLL,wl_fixed_from_double(x));
        wl_pointer_send_axis(res,time,WL_POINTER_AXIS_VERTICAL_SCROLL,wl_fixed_from_double(y));
    }

}

const LPoint &LPointer::scrollWheelStep() const
{
    return imp()->axisDiscreteStep;
}

void LPointer::setScrollWheelStep(const LPoint &step)
{
    imp()->axisDiscreteStep = step;
}

#else
void LPointer::sendAxisEvent(double x, double y)
{
    // If no surface has focus
    if(!focusSurface())
        return;

    // If do not have a wl_pointer
    wl_resource *res = focusSurface()->client()->pointerResource();
    if(!res)
        return;

    UInt32 time = LTime::ms();
    wl_pointer_send_axis(res,time,WL_POINTER_AXIS_HORIZONTAL_SCROLL,wl_fixed_from_double(x));
    wl_pointer_send_axis(res,time,WL_POINTER_AXIS_VERTICAL_SCROLL,wl_fixed_from_double(y));
}
#endif

LSurface *LPointer::surfaceAtC(const LPoint &point)
{
    for(list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin(); s != compositor()->surfaces().rend(); s++)
        if((*s)->mapped() && !(*s)->minimized())
            if((*s)->inputRegionC().containsPoint(point - (*s)->rolePosC()))
                return *s;

    return nullptr;
}

LPointer::LPointerPrivate *LPointer::imp() const
{
    return m_imp;
}

LSurface *LPointer::focusSurface() const
{
    return imp()->pointerFocusSurface;
}

void LPointer::LPointerPrivate::sendLeaveEvent(LSurface *surface)
{
    if(seat->dndManager()->focus() && seat->dndManager()->focus()->client()->dataDevice())
        seat->dndManager()->focus()->client()->dataDevice()->imp()->sendDNDLeaveEvent();

    // If surface is nullptr
    if(!surface)
        return;

    // If do not have a wl_pointer
    if(!surface->client()->pointerResource())
        return;

    // Send the unset focus event
    lastPointerLeaveEventSerial = LWayland::nextSerial();
    surface->client()->imp()->serials.lastPointerLeaveEventSerial = lastPointerLeaveEventSerial;

    wl_pointer_send_leave(surface->client()->pointerResource(),
                          surface->client()->imp()->serials.lastPointerLeaveEventSerial,
                          surface->resource());

#if LOUVRE_SEAT_VERSION >=5
    if(wl_resource_get_version(surface->client()->pointerResource()) >= 5)
        wl_pointer_send_frame(surface->client()->pointerResource());
#endif

}

void LPointer::LPointerPrivate::sendEnterEvent(LSurface *surface, const LPoint &point)
{
    // If surface is nullptr
    if(!surface)
        return;

    // If do not have a wl_pointer
    if(!surface->client()->pointerResource())
        return;

    // Send focus event
    lastPointerEnterEventSerial = LWayland::nextSerial();
    surface->client()->imp()->serials.lastPointerEnterEventSerial = lastPointerEnterEventSerial;
    wl_pointer_send_enter(surface->client()->pointerResource(),
                          lastPointerEnterEventSerial,
                          surface->resource(),
                          wl_fixed_from_double(point.x()/seat->compositor()->globalScale()),
                          wl_fixed_from_double(point.y()/seat->compositor()->globalScale()));

#if LOUVRE_SEAT_VERSION >=5
    if(wl_resource_get_version(surface->client()->pointerResource()) >= 5)
        wl_pointer_send_frame(surface->client()->pointerResource());
#endif
}






