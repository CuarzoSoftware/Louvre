#include <CZ/Louvre/Protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <CZ/Louvre/Protocols/PointerGestures/GPointerGestures.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGestureSwipe.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RPointer.h>
#include <CZ/Core/Events/CZPointerSwipeUpdateEvent.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::PointerGestures;

static const struct zwp_pointer_gesture_swipe_v1_interface imp
{
    .destroy = &RGestureSwipe::destroy
};

RGestureSwipe::RGestureSwipe(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept :
    LResource(
        pointerRes->client(),
        &zwp_pointer_gesture_swipe_v1_interface,
        version,
        id,
        &imp),
    m_pointerRes(pointerRes)
{
    pointerRes->m_gestureSwipeRes.emplace_back(this);
}

RGestureSwipe::~RGestureSwipe() noexcept
{
    if (pointerRes())
        CZVectorUtils::RemoveOneUnordered(pointerRes()->m_gestureSwipeRes, this);
}

void RGestureSwipe::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RGestureSwipe::begin(const CZPointerSwipeBeginEvent &event, Wayland::RWlSurface *surfaceRes) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.pointer.swipeBegin };

    if (clientEvent.serial != event.serial)
        clientEvent = event;

    zwp_pointer_gesture_swipe_v1_send_begin(resource(), event.serial, event.ms, surfaceRes->resource(), event.fingers);
}

void RGestureSwipe::update(const CZPointerSwipeUpdateEvent &event) noexcept
{
    zwp_pointer_gesture_swipe_v1_send_update(
        resource(),
        event.ms,
        wl_fixed_from_double(event.delta.x()),
        wl_fixed_from_double(event.delta.y()));
}

void RGestureSwipe::end(const CZPointerSwipeEndEvent &event) noexcept
{
    auto &clientPointerEvents { client()->imp()->eventHistory.pointer };

    if (clientPointerEvents.swipeEnd.serial != event.serial)
        clientPointerEvents.swipeEnd = event;

    if (event.fingers == 0)
        clientPointerEvents.swipeEnd.fingers = clientPointerEvents.swipeBegin.fingers;

    if (!clientPointerEvents.swipeEnd.device)
        clientPointerEvents.swipeEnd.device = clientPointerEvents.swipeBegin.device;

    zwp_pointer_gesture_swipe_v1_send_end(
        resource(),
        event.serial,
        event.ms,
        event.cancelled);
}
