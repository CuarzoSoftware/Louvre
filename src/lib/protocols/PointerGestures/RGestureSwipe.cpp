#include <protocols/PointerGestures/private/RGestureSwipePrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <LPointerSwipeUpdateEvent.h>
#include <private/LCompositorPrivate.h>

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
        &zwp_pointer_gesture_swipe_v1_implementation,
        &RGestureSwipe::RGestureSwipePrivate::resource_destroy),
    LPRIVATE_INIT_UNIQUE(RGestureSwipe)
{
    imp()->rPointer = rPointer;
    rPointer->imp()->rGestureSwipe = this;
}

RGestureSwipe::~RGestureSwipe()
{
    if (pointerResource())
        pointerResource()->imp()->rGestureSwipe = nullptr;
}

RPointer *RGestureSwipe::pointerResource() const
{
    return imp()->rPointer;
}

const RGestureSwipe::SerialEvents &RGestureSwipe::serialEvents() const
{
    return imp()->serialEvents;
}

bool RGestureSwipe::begin(const LPointerSwipeBeginEvent &event, Wayland::RSurface *rSurface)
{
    imp()->serialEvents.begin = event;
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
    imp()->serialEvents.end = event;

    if (event.fingers() == 0)
        imp()->serialEvents.end.setFingers(imp()->serialEvents.begin.fingers());

    if (event.device() == &compositor()->imp()->fakeDevice)
        imp()->serialEvents.end.setDevice(imp()->serialEvents.begin.device());

    zwp_pointer_gesture_swipe_v1_send_end(
        resource(),
        event.serial(),
        event.ms(),
        event.cancelled());
    return true;
}
