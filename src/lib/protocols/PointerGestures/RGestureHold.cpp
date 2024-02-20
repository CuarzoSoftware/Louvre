#include <protocols/PointerGestures/private/RGestureHoldPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>

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
    rPointer->imp()->gestureHoldResources.push_back(this);
}

RGestureHold::~RGestureHold()
{
    if (pointerResource())
        LVectorRemoveOneUnordered(pointerResource()->imp()->gestureHoldResources, this);
}

RPointer *RGestureHold::pointerResource() const
{
    return imp()->rPointer;
}

bool RGestureHold::begin(const LPointerHoldBeginEvent &event, Wayland::RSurface *rSurface)
{
    auto &clientEvent = client()->imp()->events.pointer.holdBegin;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    zwp_pointer_gesture_hold_v1_send_begin(resource(), event.serial(), event.ms(), rSurface->resource(), event.fingers());
    return true;
}

bool RGestureHold::end(const LPointerHoldEndEvent &event)
{
    auto &clientPointerEvents = client()->imp()->events.pointer;

    if (clientPointerEvents.holdEnd.serial() != event.serial())
        clientPointerEvents.holdEnd = event;

    if (event.fingers() == 0)
        clientPointerEvents.holdEnd.setFingers(clientPointerEvents.holdBegin.fingers());

    if (event.device() == &compositor()->imp()->fakeDevice)
        clientPointerEvents.holdEnd.setDevice(clientPointerEvents.holdBegin.device());

    zwp_pointer_gesture_hold_v1_send_end(
        resource(),
        event.serial(),
        event.ms(),
        event.cancelled());
    return true;
}
