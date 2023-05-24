#include <protocols/Wayland/private/GSeatPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>

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
    .set_cursor = &RPointer::RPointerPrivate::set_cursor,
#if LOUVRE_SEAT_VERSION >= WL_POINTER_RELEASE_SINCE_VERSION
    .release = &RPointer::RPointerPrivate::release
#endif
};

RPointer::RPointer(GSeat *seatGlobal, Int32 id) :
    LResource(seatGlobal->client(),
              &wl_pointer_interface,
              seatGlobal->version(),
              id,
              &pointer_implementation,
              &RPointer::RPointerPrivate::resource_destroy)
{
    m_imp = new RPointerPrivate();
    imp()->seatGlobal = seatGlobal;
    seatGlobal->imp()->pointerResource = this;
}

RPointer::~RPointer()
{
    if (seatGlobal())
        seatGlobal()->imp()->pointerResource = nullptr;
}

void RPointer::sendEnter(LSurface *surface, const LPoint &point)
{
    imp()->serials.enter = LWayland::nextSerial();

    wl_pointer_send_enter(resource(),
                          serials().enter,
                         surface->surfaceResource()->resource(),
                          wl_fixed_from_double(point.x()/compositor()->globalScale()),
                          wl_fixed_from_double(point.y()/compositor()->globalScale()));
}

void RPointer::sendLeave(LSurface *surface)
{
    imp()->serials.leave = LWayland::nextSerial();
    wl_pointer_send_leave(resource(),
                          serials().leave,
                          surface->surfaceResource()->resource());
}

void RPointer::sendFrame()
{
#if LOUVRE_SEAT_VERSION >= WL_POINTER_FRAME_SINCE_VERSION
    if (version() >= WL_POINTER_FRAME_SINCE_VERSION)
        wl_pointer_send_frame(resource());
#endif
}

void RPointer::sendAxis(double x, double y, UInt32 source)
{
    UInt32 time = LTime::ms();

    if (version() >= WL_POINTER_AXIS_SOURCE_SINCE_VERSION)
    {
        if (source == WL_POINTER_AXIS_SOURCE_WHEEL || source == WL_POINTER_AXIS_SOURCE_WHEEL_TILT)
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
            if (x == 0.0)
                wl_pointer_send_axis_stop(resource(), time, WL_POINTER_AXIS_HORIZONTAL_SCROLL);
            else
                wl_pointer_send_axis(resource(), time, WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_double(x));

            if (y == 0.0)
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

void RPointer::sendAxis(double x, double y)
{
    UInt32 time = LTime::ms();
    wl_pointer_send_axis(resource(), time, WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_double(x));
    wl_pointer_send_axis(resource(), time, WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_double(y));
}

void RPointer::sendMove(const LPoint &localPos)
{
    wl_pointer_send_motion(resource(),
                           LTime::ms(),
                           wl_fixed_from_int(localPos.x()/compositor()->globalScale()),
                           wl_fixed_from_int(localPos.y()/compositor()->globalScale()));
}

void RPointer::sendButton(LPointer::Button button, LPointer::ButtonState state)
{
    imp()->serials.button = LWayland::nextSerial();
    wl_pointer_send_button(resource(),
                           serials().button,
                           LTime::ms(),
                           button,
                           state);

}

GSeat *RPointer::seatGlobal() const
{
    return imp()->seatGlobal;
}

const RPointer::LastEventSerials &RPointer::serials() const
{
    return imp()->serials;
}
