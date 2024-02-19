#include <protocols/PointerGestures/private/GPointerGesturesPrivate.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <protocols/PointerGestures/RGestureSwipe.h>
#include <protocols/PointerGestures/RGesturePinch.h>
#include <protocols/PointerGestures/RGestureHold.h>
#include <protocols/Wayland/RPointer.h>

struct zwp_pointer_gestures_v1_interface zwp_pointer_gestures_v1_implementation =
{
    .get_swipe_gesture = &GPointerGestures::GPointerGesturesPrivate::get_swipe_gesture,
    .get_pinch_gesture = &GPointerGestures::GPointerGesturesPrivate::get_pinch_gesture,

    #if LOUVRE_POINTER_GESTURES_VERSION >= 2
    .release = &GPointerGestures::GPointerGesturesPrivate::release,
    #endif

    #if LOUVRE_POINTER_GESTURES_VERSION >= 3
    .get_hold_gesture = &GPointerGestures::GPointerGesturesPrivate::get_hold_gesture
    #endif
};

void GPointerGestures::GPointerGesturesPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data)
    new GPointerGestures(client,
                         &zwp_pointer_gestures_v1_interface,
                         version,
                         id,
                         &zwp_pointer_gestures_v1_implementation,
                         &GPointerGestures::GPointerGesturesPrivate::resource_destroy);
}

void GPointerGestures::GPointerGesturesPrivate::resource_destroy(wl_resource *resource)
{
    GPointerGestures *gPointerGestures = (GPointerGestures*)wl_resource_get_user_data(resource);
    delete gPointerGestures;
}

void GPointerGestures::GPointerGesturesPrivate::get_swipe_gesture(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer)
{
    L_UNUSED(client)
    Wayland::RPointer *rPointer = (Wayland::RPointer*)wl_resource_get_user_data(pointer);

    if (rPointer->gestureSwipeResource())
        return;

    new RGestureSwipe(rPointer, id, wl_resource_get_version(resource));
}

void GPointerGestures::GPointerGesturesPrivate::get_pinch_gesture(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer)
{
    L_UNUSED(client)
    Wayland::RPointer *rPointer = (Wayland::RPointer*)wl_resource_get_user_data(pointer);

    if (rPointer->gesturePinchResource())
        return;

    new RGesturePinch(rPointer, id, wl_resource_get_version(resource));
}

#if LOUVRE_POINTER_GESTURES_VERSION >= 2
void GPointerGestures::GPointerGesturesPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client)
    wl_resource_destroy(resource);
}
#endif

#if LOUVRE_POINTER_GESTURES_VERSION >= 3
void GPointerGestures::GPointerGesturesPrivate::get_hold_gesture(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer)
{
    L_UNUSED(client)
    Wayland::RPointer *rPointer = (Wayland::RPointer*)wl_resource_get_user_data(pointer);

    if (rPointer->gestureHoldResource())
        return;

    new RGestureHold(rPointer, id, wl_resource_get_version(resource));
}
#endif
