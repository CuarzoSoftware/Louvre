#include <protocols/PointerGestures/private/RGesturePinchPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <LPointerPinchUpdateEvent.h>
#include <private/LCompositorPrivate.h>

static struct zwp_pointer_gesture_pinch_v1_interface zwp_pointer_gesture_pinch_v1_implementation =
{
    .destroy = &RGesturePinch::RGesturePinchPrivate::destroy
};

RGesturePinch::RGesturePinch(Wayland::RPointer *rPointer, Int32 id, UInt32 version) :
    LResource(
        rPointer->client(),
        &zwp_pointer_gesture_pinch_v1_interface,
        version,
        id,
        &zwp_pointer_gesture_pinch_v1_implementation,
        &RGesturePinch::RGesturePinchPrivate::resource_destroy),
    LPRIVATE_INIT_UNIQUE(RGesturePinch)
{
    imp()->rPointer = rPointer;
    rPointer->imp()->rGesturePinch = this;
}

RGesturePinch::~RGesturePinch()
{
    if (pointerResource())
        pointerResource()->imp()->rGesturePinch = nullptr;
}

RPointer *RGesturePinch::pointerResource() const
{
    return imp()->rPointer;
}

const RGesturePinch::SerialEvents &RGesturePinch::serialEvents() const
{
    return imp()->serialEvents;
}

bool RGesturePinch::begin(const LPointerPinchBeginEvent &event, Wayland::RSurface *rSurface)
{
    imp()->serialEvents.begin = event;
    zwp_pointer_gesture_pinch_v1_send_begin(resource(), event.serial(), event.ms(), rSurface->resource(), event.fingers());
    return true;
}

bool RGesturePinch::update(const LPointerPinchUpdateEvent &event)
{
    zwp_pointer_gesture_pinch_v1_send_update(
        resource(),
        event.ms(),
        wl_fixed_from_double(event.delta().x()),
        wl_fixed_from_double(event.delta().y()),
        wl_fixed_from_double(event.scale()),
        wl_fixed_from_double(event.rotation()));
    return true;
}

bool RGesturePinch::end(const LPointerPinchEndEvent &event)
{
    imp()->serialEvents.end = event;

    if (event.fingers() == 0)
        imp()->serialEvents.end.setFingers(imp()->serialEvents.begin.fingers());

    if (event.device() == &compositor()->imp()->fakeDevice)
        imp()->serialEvents.end.setDevice(imp()->serialEvents.begin.device());

    zwp_pointer_gesture_pinch_v1_send_end(
        resource(),
        event.serial(),
        event.ms(),
        event.cancelled());
    return true;
}
