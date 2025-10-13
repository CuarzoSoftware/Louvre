#include <CZ/Louvre/Protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <CZ/Louvre/Protocols/PointerGestures/GPointerGestures.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGestureSwipe.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGesturePinch.h>
#include <CZ/Louvre/Protocols/PointerGestures/RGestureHold.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::PointerGestures;

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

LGLOBAL_INTERFACE_IMP(GPointerGestures, LOUVRE_POINTER_GESTURES_VERSION, zwp_pointer_gestures_v1_interface)

bool GPointerGestures::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.PointerGestures)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.PointerGestures;
    return true;
}

GPointerGestures::GPointerGestures
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->pointerGesturesGlobals.push_back(this);
}

GPointerGestures::~GPointerGestures() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->pointerGesturesGlobals, this);
}

/******************** REQUESTS ********************/

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
