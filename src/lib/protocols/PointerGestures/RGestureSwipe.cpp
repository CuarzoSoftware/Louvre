#include <protocols/PointerGestures/private/RGestureSwipePrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <LPointerSwipeUpdateEvent.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>

static struct zwp_pointer_gesture_swipe_v1_interface zwp_pointer_gesture_swipe_v1_implementation =
{
    .destroy = &RGestureSwipe::RGestureSwipePrivate::destroy
};

RGestureSwipe::RGestureSwipe(Wayland::RPointer *rPointer, Int32 id, UInt32 version) :
    LResource(
        rPointer->client(),
        &zwp_pointer_gesture_swipe_v1_interface,
        version,
        id,
        &zwp_pointer_gesture_swipe_v1_implementation),
    LPRIVATE_INIT_UNIQUE(RGestureSwipe)
{
    imp()->rPointer = rPointer;
    rPointer->imp()->gestureSwipeResources.push_back(this);
}

RGestureSwipe::~RGestureSwipe()
{
    if (pointerResource())
        LVectorRemoveOneUnordered(pointerResource()->imp()->gestureSwipeResources, this);
}

RPointer *RGestureSwipe::pointerResource() const
{
    return imp()->rPointer;
}

bool RGestureSwipe::begin(const LPointerSwipeBeginEvent &event, Wayland::RSurface *rSurface)
{
    auto &clientEvent = client()->imp()->events.pointer.swipeBegin;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    zwp_pointer_gesture_swipe_v1_send_begin(resource(), event.serial(), event.ms(), rSurface->resource(), event.fingers());
    return true;
}

bool RGestureSwipe::update(const LPointerSwipeUpdateEvent &event)
{
    zwp_pointer_gesture_swipe_v1_send_update(
        resource(),
        event.ms(),
        wl_fixed_from_double(event.delta().x()),
        wl_fixed_from_double(event.delta().y()));
    return true;
}

bool RGestureSwipe::end(const LPointerSwipeEndEvent &event)
{
    auto &clientPointerEvents = client()->imp()->events.pointer;

    if (clientPointerEvents.swipeEnd.serial() != event.serial())
        clientPointerEvents.swipeEnd = event;

    if (event.fingers() == 0)
        clientPointerEvents.swipeEnd.setFingers(clientPointerEvents.swipeBegin.fingers());

    if (event.device() == &compositor()->imp()->fakeDevice)
        clientPointerEvents.swipeEnd.setDevice(clientPointerEvents.swipeBegin.device());

    zwp_pointer_gesture_swipe_v1_send_end(
        resource(),
        event.serial(),
        event.ms(),
        event.cancelled());
    return true;
}
