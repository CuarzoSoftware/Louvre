#ifndef GPOINTERGESTURES_H
#define GPOINTERGESTURES_H

#include <LResource.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>

class Louvre::Protocols::PointerGestures::GPointerGestures final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void get_swipe_gesture(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer) noexcept;
    static void get_pinch_gesture(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer) noexcept;

#if LOUVRE_POINTER_GESTURES_VERSION >= 2
    static void release(wl_client *client, wl_resource *resource) noexcept;
#endif

#if LOUVRE_POINTER_GESTURES_VERSION >= 3
    static void get_hold_gesture(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer) noexcept;
#endif

private:
    GPointerGestures(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GPointerGestures() noexcept;
};

#endif // GPOINTERGESTURES_H
