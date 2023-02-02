#include <protocols/Wayland/private/SeatGlobalPrivate.h>
#include <protocols/Wayland/private/PointerResourcePrivate.h>

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
    .set_cursor = &PointerResource::PointerResourcePrivate::set_cursor,
#if LOUVRE_SEAT_VERSION >= WL_POINTER_RELEASE_SINCE_VERSION
    .release = &PointerResource::PointerResourcePrivate::release
#endif
};

PointerResource::PointerResource(SeatGlobal *seatGlobal, Int32 id) :
    LResource(seatGlobal->client(),
              &wl_pointer_interface,
              seatGlobal->version(),
              id,
              &pointer_implementation,
              &PointerResource::PointerResourcePrivate::resource_destroy)
{
    m_imp = new PointerResourcePrivate();
    imp()->seatGlobal = seatGlobal;
    seatGlobal->imp()->pointerResource = this;
}

PointerResource::~PointerResource()
{
    if(seatGlobal())
        seatGlobal()->imp()->pointerResource = nullptr;
}

void PointerResource::sendEnter(LSurface *surface, const LPoint &point)
{
    imp()->serials.enter = LWayland::nextSerial();

    wl_pointer_send_enter(resource(),
                          serials().enter,
                          surface->resource(),
                          wl_fixed_from_double(point.x()/compositor()->globalScale()),
                          wl_fixed_from_double(point.y()/compositor()->globalScale()));
}

void PointerResource::sendLeave(LSurface *surface)
{
    imp()->serials.leave = LWayland::nextSerial();
    wl_pointer_send_leave(resource(),
                          serials().leave,
                          surface->resource());
}

void PointerResource::sendFrame()
{
#if LOUVRE_SEAT_VERSION >= WL_POINTER_FRAME_SINCE_VERSION
    if(version() >= WL_POINTER_FRAME_SINCE_VERSION)
        wl_pointer_send_frame(resource());
#endif
}

void PointerResource::sendAxis(double x, double y, UInt32 source)
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

void PointerResource::sendAxis(double x, double y)
{
    UInt32 time = LTime::ms();
    wl_pointer_send_axis(resource(), time, WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_double(x));
    wl_pointer_send_axis(resource(), time, WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_double(y));
}

void PointerResource::sendMove(const LPoint &localPos)
{
    wl_pointer_send_motion(resource(),
                           LTime::ms(),
                           wl_fixed_from_int(localPos.x()/compositor()->globalScale()),
                           wl_fixed_from_int(localPos.y()/compositor()->globalScale()));
}

void PointerResource::sendButton(LPointer::Button button, LPointer::ButtonState state)
{
    imp()->serials.button = LWayland::nextSerial();
    wl_pointer_send_button(resource(),
                           serials().button,
                           LTime::ms(),
                           button,
                           state);

}

SeatGlobal *PointerResource::seatGlobal() const
{
    return imp()->seatGlobal;
}

const PointerResource::LastEventSerials &PointerResource::serials() const
{
    return imp()->serials;
}

PointerResource::PointerResourcePrivate *PointerResource::imp() const
{
    return m_imp;
}



