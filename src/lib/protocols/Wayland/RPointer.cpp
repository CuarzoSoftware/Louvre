#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/private/GSeatPrivate.h>

static struct wl_pointer_interface pointer_implementation =
{
    .set_cursor = &RPointer::RPointerPrivate::set_cursor,
#if LOUVRE_WL_SEAT_VERSION >= 3
    .release = &RPointer::RPointerPrivate::release
#endif
};

RPointer::RPointer
(
    GSeat *gSeat,
    Int32 id
)
    :LResource
    (
        gSeat->client(),
        &wl_pointer_interface,
        gSeat->version(),
        id,
        &pointer_implementation,
        &RPointer::RPointerPrivate::resource_destroy
    )
{
    m_imp = new RPointerPrivate();
    imp()->gSeat = gSeat;
    gSeat->imp()->rPointer = this;
}

RPointer::~RPointer()
{
    if (seatGlobal())
        seatGlobal()->imp()->rPointer = nullptr;

    delete m_imp;
}

GSeat *RPointer::seatGlobal() const
{
    return imp()->gSeat;
}

const RPointer::LastEventSerials &RPointer::serials() const
{
    return imp()->serials;
}

bool RPointer::enter(UInt32 serial, RSurface *rSurface, Float24 x, Float24 y)
{
    wl_pointer_send_enter(resource(),
                          serial,
                          rSurface->resource(),
                          x,
                          y);
    return true;
}

bool RPointer::leave(UInt32 serial, RSurface *rSurface)
{
    wl_pointer_send_leave(resource(), serial, rSurface->resource());
    return true;
}

bool RPointer::motion(UInt32 time, Float24 x, Float24 y)
{
    wl_pointer_send_motion(resource(), time, x, y);
    return true;
}

bool RPointer::button(UInt32 serial, UInt32 time, UInt32 button, UInt32 state)
{
    wl_pointer_send_button(resource(), serial, time, button, state);
    return true;
}

bool RPointer::axis(UInt32 time, UInt32 axis, Float24 value)
{
    wl_pointer_send_axis(resource(), time, axis, value);
    return true;
}

bool RPointer::frame()
{
#if LOUVRE_WL_SEAT_VERSION >= 5
    if (version() >= 5)
    {
        wl_pointer_send_frame(resource());
        return true;
    }
#endif
    return false;
}

bool RPointer::axisSource(UInt32 axisSource)
{
#if LOUVRE_WL_SEAT_VERSION >= 5
    if (version() >= 5)
    {
        wl_pointer_send_axis_source(resource(), axisSource);
        return true;
    }
#endif
    L_UNUSED(axisSource);
    return false;
}

bool RPointer::axisStop(UInt32 time, UInt32 axis)
{
#if LOUVRE_WL_SEAT_VERSION >= 5
    if (version() >= 5)
    {
        wl_pointer_send_axis_stop(resource(), time, axis);
        return true;
    }
#endif
    L_UNUSED(time);
    L_UNUSED(axis);
    return false;
}

bool RPointer::axisDiscrete(UInt32 axis, Int32 discrete)
{
#if LOUVRE_WL_SEAT_VERSION >= 5
    if (version() >= 5)
    {
        wl_pointer_send_axis_discrete(resource(), axis, discrete);
        return true;
    }
#endif
    L_UNUSED(axis);
    L_UNUSED(discrete);
    return false;
}

bool RPointer::axisValue120(UInt32 axis, Int32 value120)
{
#if LOUVRE_WL_SEAT_VERSION >= 8
    if (version() >= 8)
    {
        wl_pointer_send_axis_value120(resource(), axis, value120);
        return true;
    }
#endif
    L_UNUSED(axis);
    L_UNUSED(value120);
    return false;
}

bool RPointer::axisRelativeDirection(UInt32 axis, UInt32 direction)
{
#if LOUVRE_WL_SEAT_VERSION >= 9
    if (version() >= 9)
    {
        wl_pointer_send_axis_relative_direction(resource(), axis, direction);
        return true;
    }
#endif
    L_UNUSED(axis);
    L_UNUSED(direction);
    return false;
}
