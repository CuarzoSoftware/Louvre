#include <protocols/PointerGestures/GPointerGestures.h>
#include <protocols/PointerGestures/RGestureSwipe.h>
#include <protocols/PointerGestures/RGesturePinch.h>
#include <protocols/PointerGestures/RGestureHold.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::PointerGestures;

static const struct zwp_pointer_gestures_v1_interface imp
{
    .get_swipe_gesture = &GPointerGestures::get_swipe_gesture,
    .get_pinch_gesture = &GPointerGestures::get_pinch_gesture,

#if LOUVRE_POINTER_GESTURES_VERSION >= 2
    .release = &GPointerGestures::release,
#endif

#if LOUVRE_POINTER_GESTURES_VERSION >= 3
    .get_hold_gesture = &GPointerGestures::get_hold_gesture
#endif
};

GPointerGestures::GPointerGestures
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        client,
        &zwp_pointer_gestures_v1_interface,
        version,
        id,
        &imp
    )
{
    this->client()->imp()->pointerGesturesGlobals.push_back(this);
}

GPointerGestures::~GPointerGestures() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->pointerGesturesGlobals, this);
}

/******************** REQUESTS ********************/

void GPointerGestures::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GPointerGestures(client, version, id);
}

void GPointerGestures::get_swipe_gesture(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *pointer) noexcept
{
    new RGestureSwipe(static_cast<Wayland::RPointer*>(wl_resource_get_user_data(pointer)),
                      id,
                      wl_resource_get_version(resource));
}

void GPointerGestures::get_pinch_gesture(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *pointer) noexcept
{
    new RGesturePinch(static_cast<Wayland::RPointer*>(wl_resource_get_user_data(pointer)),
                      id,
                      wl_resource_get_version(resource));
}

#if LOUVRE_POINTER_GESTURES_VERSION >= 2
void GPointerGestures::release(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
#endif

#if LOUVRE_POINTER_GESTURES_VERSION >= 3
void GPointerGestures::get_hold_gesture(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *pointer) noexcept
{
    new RGestureHold(static_cast<Wayland::RPointer*>(wl_resource_get_user_data(pointer)),
                     id,
                     wl_resource_get_version(resource));
}
#endif
