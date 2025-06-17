#include <CZ/Louvre/Protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <CZ/Louvre/Protocols/PointerGestures/GPointerGestures.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGesturePinch.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RPointer.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Events/LPointerPinchUpdateEvent.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::PointerGestures;

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
        LVectorRemoveOneUnordered(pointerRes()->m_gesturePinchRes, this);
}

/******************** REQUESTS ********************/

void RGesturePinch::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RGesturePinch::begin(const LPointerPinchBeginEvent &event, Wayland::RSurface *surfaceRes) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.pointer.pinchBegin };

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    zwp_pointer_gesture_pinch_v1_send_begin(resource(), event.serial(), event.ms(), surfaceRes->resource(), event.fingers());
}

void RGesturePinch::update(const LPointerPinchUpdateEvent &event) noexcept
{
    zwp_pointer_gesture_pinch_v1_send_update(
        resource(),
        event.ms(),
        wl_fixed_from_double(event.delta().x()),
        wl_fixed_from_double(event.delta().y()),
        wl_fixed_from_double(event.scale()),
        wl_fixed_from_double(event.rotation()));
}

void RGesturePinch::end(const LPointerPinchEndEvent &event) noexcept
{
    auto &clientPointerEvents { client()->imp()->eventHistory.pointer };

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
}
