#include <protocols/PointerGestures/private/RGestureHoldPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <private/LCompositorPrivate.h>

static struct zwp_pointer_gesture_hold_v1_interface zwp_pointer_gesture_hold_v1_implementation =
{
    .destroy = &RGestureHold::RGestureHoldPrivate::destroy
};

RGestureHold::RGestureHold(Wayland::RPointer *rPointer, Int32 id, UInt32 version) :
    LResource(
        rPointer->client(),
        &zwp_pointer_gesture_hold_v1_interface,
        version,
        id,
        &zwp_pointer_gesture_hold_v1_implementation,
        &RGestureHold::RGestureHoldPrivate::resource_destroy),
    LPRIVATE_INIT_UNIQUE(RGestureHold)
{
    imp()->rPointer = rPointer;
    rPointer->imp()->rGestureHold = this;
}

RGestureHold::~RGestureHold()
{
    if (pointerResource())
        pointerResource()->imp()->rGestureHold = nullptr;
}

RPointer *RGestureHold::pointerResource() const
{
    return imp()->rPointer;
}

const RGestureHold::SerialEvents &RGestureHold::serialEvents() const
{
    return imp()->serialEvents;
}

bool RGestureHold::begin(const LPointerHoldBeginEvent &event, Wayland::RSurface *rSurface)
{
    imp()->serialEvents.begin = event;
    zwp_pointer_gesture_hold_v1_send_begin(resource(), event.serial(), event.ms(), rSurface->resource(), event.fingers());
    return true;
}

bool RGestureHold::end(const LPointerHoldEndEvent &event)
{
    imp()->serialEvents.end = event;

    if (event.fingers() == 0)
        imp()->serialEvents.end.setFingers(imp()->serialEvents.begin.fingers());

    if (event.device() == &compositor()->imp()->fakeDevice)
        imp()->serialEvents.end.setDevice(imp()->serialEvents.begin.device());

    zwp_pointer_gesture_hold_v1_send_end(
        resource(),
        event.serial(),
        event.ms(),
        event.cancelled());
    return true;
}
