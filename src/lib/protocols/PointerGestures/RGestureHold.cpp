#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <protocols/PointerGestures/GPointerGestures.h>
#include <protocols/PointerGestures/RGestureHold.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/Wayland/RPointer.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::PointerGestures;

static const struct zwp_pointer_gesture_hold_v1_interface imp
{
    .destroy = &RGestureHold::destroy
};

RGestureHold::RGestureHold(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept :
    LResource(
        pointerRes->client(),
        &zwp_pointer_gesture_hold_v1_interface,
        version,
        id,
        &imp),
    m_pointerRes(pointerRes)
{
    pointerRes->m_gestureHoldRes.emplace_back(this);
}

RGestureHold::~RGestureHold() noexcept
{
    if (pointerRes())
        LVectorRemoveOneUnordered(pointerRes()->m_gestureHoldRes, this);
}

/******************** REQUESTS ********************/

void RGestureHold::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RGestureHold::begin(const LPointerHoldBeginEvent &event, Wayland::RSurface *surfaceRes) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.pointer.holdBegin };

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    zwp_pointer_gesture_hold_v1_send_begin(resource(), event.serial(), event.ms(), surfaceRes->resource(), event.fingers());
}

void RGestureHold::end(const LPointerHoldEndEvent &event) noexcept
{
    auto &clientPointerEvents { client()->imp()->eventHistory.pointer };

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
}
