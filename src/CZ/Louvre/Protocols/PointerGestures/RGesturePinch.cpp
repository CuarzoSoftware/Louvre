#include <CZ/Louvre/Protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <CZ/Louvre/Protocols/PointerGestures/GPointerGestures.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGesturePinch.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RPointer.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Core/Events/CZPointerPinchUpdateEvent.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::PointerGestures;

static const struct zwp_pointer_gesture_pinch_v1_interface imp
{
    .destroy = &RGesturePinch::destroy
};

RGesturePinch::RGesturePinch(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept :
    LResource(
        pointerRes->client(),
        &zwp_pointer_gesture_pinch_v1_interface,
        version,
        id,
        &imp),
    m_pointerRes(pointerRes)
{
    pointerRes->m_gesturePinchRes.emplace_back(this);
}

RGesturePinch::~RGesturePinch() noexcept
{
    if (pointerRes())
        CZVectorUtils::RemoveOneUnordered(pointerRes()->m_gesturePinchRes, this);
}

/******************** REQUESTS ********************/

void RGesturePinch::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RGesturePinch::begin(const CZPointerPinchBeginEvent &event, Wayland::RWlSurface *surfaceRes) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.pointer.pinchBegin };

    if (clientEvent.serial != event.serial)
        clientEvent = event;

    zwp_pointer_gesture_pinch_v1_send_begin(resource(), event.serial, event.ms, surfaceRes->resource(), event.fingers);
}

void RGesturePinch::update(const CZPointerPinchUpdateEvent &event) noexcept
{
    zwp_pointer_gesture_pinch_v1_send_update(
        resource(),
        event.ms,
        wl_fixed_from_double(event.delta.x()),
        wl_fixed_from_double(event.delta.y()),
        wl_fixed_from_double(event.scale),
        wl_fixed_from_double(event.rotation));
}

void RGesturePinch::end(const CZPointerPinchEndEvent &event) noexcept
{
    auto &clientPointerEvents { client()->imp()->eventHistory.pointer };

    if (clientPointerEvents.pinchEnd.serial != event.serial)
        clientPointerEvents.pinchEnd = event;

    if (event.fingers == 0)
        clientPointerEvents.pinchEnd.fingers = clientPointerEvents.pinchBegin.fingers;

    if (!clientPointerEvents.pinchEnd.device)
        clientPointerEvents.pinchEnd.device = clientPointerEvents.pinchBegin.device;

    zwp_pointer_gesture_pinch_v1_send_end(
        resource(),
        event.serial,
        event.ms,
        event.cancelled);
}
