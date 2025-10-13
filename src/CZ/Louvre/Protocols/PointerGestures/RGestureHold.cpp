#include <CZ/Louvre/Protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <CZ/Louvre/Protocols/PointerGestures/GPointerGestures.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGestureHold.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RPointer.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::PointerGestures;

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
        CZVectorUtils::RemoveOneUnordered(pointerRes()->m_gestureHoldRes, this);
}

/******************** REQUESTS ********************/

void RGestureHold::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RGestureHold::begin(const CZPointerHoldBeginEvent &event, Wayland::RWlSurface *surfaceRes) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.pointer.holdBegin };

    if (clientEvent.serial != event.serial)
        clientEvent = event;

    zwp_pointer_gesture_hold_v1_send_begin(resource(), event.serial, event.ms, surfaceRes->resource(), event.fingers);
}

void RGestureHold::end(const CZPointerHoldEndEvent &event) noexcept
{
    auto &clientPointerEvents { client()->imp()->eventHistory.pointer };

    if (clientPointerEvents.holdEnd.serial != event.serial)
        clientPointerEvents.holdEnd = event;

    if (event.fingers == 0)
        clientPointerEvents.holdEnd.fingers = clientPointerEvents.holdBegin.fingers;

    if (!clientPointerEvents.holdEnd.device)
        clientPointerEvents.holdEnd.device = clientPointerEvents.holdBegin.device;

    zwp_pointer_gesture_hold_v1_send_end(
        resource(),
        event.serial,
        event.ms,
        event.cancelled);
}
