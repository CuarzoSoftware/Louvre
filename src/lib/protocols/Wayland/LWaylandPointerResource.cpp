#include <protocols/Wayland/private/LWaylandSeatGlobalPrivate.h>
#include <protocols/Wayland/private/LWaylandPointerResourcePrivate.h>

#include <private/LCursorRolePrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSurfacePrivate.h>

#include <LWayland.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LPoint.h>
#include <LPointer.h>
#include <LTime.h>

#include <stdio.h>

static struct wl_pointer_interface pointer_implementation =
{
    .set_cursor = &LWaylandPointerResource::LWaylandPointerResourcePrivate::set_cursor,
#if LOUVRE_SEAT_VERSION >= WL_POINTER_RELEASE_SINCE_VERSION
    .release = &LWaylandPointerResource::LWaylandPointerResourcePrivate::release
#endif
};

LWaylandPointerResource::LWaylandPointerResource(LWaylandSeatGlobal *seatGlobal, Int32 id) :
    LResource(seatGlobal->client(),
              &wl_pointer_interface,
              seatGlobal->version(),
              id,
              &pointer_implementation,
              &LWaylandPointerResource::LWaylandPointerResourcePrivate::resource_destroy)
{
    m_imp = new LWaylandPointerResourcePrivate();
    imp()->seatGlobal = seatGlobal;
    seatGlobal->imp()->pointerResources.push_back(this);
    imp()->seatLink = std::prev(seatGlobal->imp()->pointerResources.end());
}

LWaylandPointerResource::~LWaylandPointerResource()
{
    if(seatGlobal())
        seatGlobal()->imp()->pointerResources.erase(imp()->seatLink);
}

void LWaylandPointerResource::sendEnter(LSurface *surface, const LPoint &point)
{
    seatGlobal()->imp()->pointerSerials.enter = LWayland::nextSerial();

    wl_pointer_send_enter(resource(),
                          seatGlobal()->pointerSerials().enter,
                          surface->resource(),
                          wl_fixed_from_double(point.x()/compositor()->globalScale()),
                          wl_fixed_from_double(point.y()/compositor()->globalScale()));
}

void LWaylandPointerResource::sendLeave(LSurface *surface)
{
    seatGlobal()->imp()->pointerSerials.leave = LWayland::nextSerial();
    wl_pointer_send_leave(resource(),
                          seatGlobal()->pointerSerials().leave,
                          surface->resource());
}

void LWaylandPointerResource::sendFrame()
{
#if LOUVRE_SEAT_VERSION >= WL_POINTER_FRAME_SINCE_VERSION
    if(version() >= WL_POINTER_FRAME_SINCE_VERSION)
        wl_pointer_send_frame(resource());
#endif
}

void LWaylandPointerResource::sendAxis(double x, double y, UInt32 source)
{
    UInt32 time = LTime::ms();

    if(version() >= WL_POINTER_AXIS_SOURCE_SINCE_VERSION)
    {
        if(source == WL_POINTER_AXIS_SOURCE_WHEEL || source == WL_POINTER_AXIS_SOURCE_WHEEL_TILT)
        {
            wl_pointer_send_axis_discrete(resource(),
                WL_POINTER_AXIS_HORIZONTAL_SCROLL,
                compositor()->seat()->pointer()->scrollWheelStep().x());

            wl_pointer_send_axis(resource(),
                time,
                WL_POINTER_AXIS_HORIZONTAL_SCROLL,
                wl_fixed_from_double(x));

            wl_pointer_send_axis_discrete(resource(),
                WL_POINTER_AXIS_VERTICAL_SCROLL,
                compositor()->seat()->pointer()->scrollWheelStep().y());

            wl_pointer_send_axis(resource(),
                time,
                WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_double(y));
        }
        else
        {
            if(x == 0.0)
                wl_pointer_send_axis_stop(resource(), time, WL_POINTER_AXIS_HORIZONTAL_SCROLL);
            else
                wl_pointer_send_axis(resource(), time, WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_double(x));

            if(y == 0.0)
                wl_pointer_send_axis_stop(resource(), time, WL_POINTER_AXIS_VERTICAL_SCROLL);
            else
                wl_pointer_send_axis(resource(), time, WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_double(y));
        }
        wl_pointer_send_axis_source(resource(), source);
        sendFrame();
    }
    else
    {
        sendAxis(x, y);
    }
}

void LWaylandPointerResource::sendAxis(double x, double y)
{
    UInt32 time = LTime::ms();
    wl_pointer_send_axis(resource(), time, WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_double(x));
    wl_pointer_send_axis(resource(), time, WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_double(y));
}

void LWaylandPointerResource::sendMove(const LPoint &localPos)
{
    wl_pointer_send_motion(resource(),
                           LTime::ms(),
                           wl_fixed_from_int(localPos.x()/compositor()->globalScale()),
                           wl_fixed_from_int(localPos.y()/compositor()->globalScale()));
}

void LWaylandPointerResource::sendButton(LPointer::Button button, LPointer::ButtonState state)
{
    // Send pointer button event
    seatGlobal()->imp()->pointerSerials.button = LWayland::nextSerial();
    wl_pointer_send_button(resource(),
                           seatGlobal()->pointerSerials().button,
                           LTime::ms(),
                           button,
                           state);

}

LWaylandSeatGlobal *LWaylandPointerResource::seatGlobal() const
{
    return imp()->seatGlobal;
}

LWaylandPointerResource::LWaylandPointerResourcePrivate *LWaylandPointerResource::imp() const
{
    return m_imp;
}



