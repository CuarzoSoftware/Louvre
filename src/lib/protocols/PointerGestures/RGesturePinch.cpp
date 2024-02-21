#include <protocols/PointerGestures/private/RGesturePinchPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <LPointerPinchUpdateEvent.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>

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
        &zwp_pointer_gesture_pinch_v1_implementation),
    LPRIVATE_INIT_UNIQUE(RGesturePinch)
{
    imp()->rPointer = rPointer;
    rPointer->imp()->gesturePinchResources.push_back(this);
}

RGesturePinch::~RGesturePinch()
{
    if (pointerResource())
        LVectorRemoveOneUnordered(pointerResource()->imp()->gesturePinchResources, this);
}

RPointer *RGesturePinch::pointerResource() const
{
    return imp()->rPointer;
}

bool RGesturePinch::begin(const LPointerPinchBeginEvent &event, Wayland::RSurface *rSurface)
{
    auto &clientEvent = client()->imp()->events.pointer.pinchBegin;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    zwp_pointer_gesture_pinch_v1_send_begin(resource(), event.serial(), event.ms(), rSurface->resource(), event.fingers());
    return true;
}

bool RGesturePinch::update(const LPointerPinchUpdateEvent &event)
{
    auto &clientEvent = client()->imp()->events.pointer.pinchUpdate;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

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
    auto &clientPointerEvents = client()->imp()->events.pointer;

    if (clientPointerEvents.pinchEnd.serial() != event.serial())
        clientPointerEvents.pinchEnd = event;

    if (event.fingers() == 0)
        clientPointerEvents.pinchEnd.setFingers(clientPointerEvents.pinchBegin.fingers());

    if (event.device() == &compositor()->imp()->fakeDevice)
        clientPointerEvents.pinchEnd.setDevice(clientPointerEvents.pinchBegin.device());

    zwp_pointer_gesture_pinch_v1_send_end(
        resource(),
        event.serial(),
        event.ms(),
        event.cancelled());
    return true;
}
