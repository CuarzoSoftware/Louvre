#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <protocols/RelativePointer/private/RRelativePointerPrivate.h>
#include <private/LClientPrivate.h>

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
    ),
    LPRIVATE_INIT_UNIQUE(RPointer)
{
    imp()->gSeat = gSeat;
    gSeat->imp()->rPointers.push_back(this);
}

RPointer::~RPointer()
{
    if (seatGlobal())
        LVectorRemoveOneUnordered(seatGlobal()->imp()->rPointers, this);

    while (!relativePointerResources().empty())
    {
        imp()->relativePointerResources.back()->imp()->rPointer = nullptr;
        imp()->relativePointerResources.pop_back();
    }
}

GSeat *RPointer::seatGlobal() const
{
    return imp()->gSeat;
}

const std::vector<RelativePointer::RRelativePointer *> &RPointer::relativePointerResources() const
{
    return imp()->relativePointerResources;
}

const std::vector<PointerGestures::RGestureSwipe *> &RPointer::gestureSwipeResources() const
{
    return imp()->gestureSwipeResources;
}

const std::vector<PointerGestures::RGesturePinch *> &RPointer::gesturePinchResources() const
{
    return imp()->gesturePinchResources;
}

const std::vector<PointerGestures::RGestureHold *> &RPointer::gestureHoldResources() const
{
    return imp()->gestureHoldResources;
}

bool RPointer::enter(const LPointerEnterEvent &event, RSurface *rSurface)
{
    auto &clientEvent = client()->imp()->events.pointer.enter;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_pointer_send_enter(resource(),
                          event.serial(),
                          rSurface->resource(),
                          wl_fixed_from_double(event.localPos.x()),
                          wl_fixed_from_double(event.localPos.y()));
    return true;
}

bool RPointer::leave(const LPointerLeaveEvent &event, RSurface *rSurface)
{
    auto &clientEvent = client()->imp()->events.pointer.leave;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_pointer_send_leave(resource(), event.serial(), rSurface->resource());
    return true;
}

bool RPointer::motion(const LPointerMoveEvent &event)
{
    auto &clientEvent = client()->imp()->events.pointer.move;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_pointer_send_motion(resource(),
                           event.ms(),
                           wl_fixed_from_double(event.localPos.x()),
                           wl_fixed_from_double(event.localPos.y()));
    return true;
}

bool RPointer::button(const LPointerButtonEvent &event)
{
    auto &clientEvent = client()->imp()->events.pointer.button;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_pointer_send_button(resource(), event.serial(), event.ms(), event.button(), event.state());
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
