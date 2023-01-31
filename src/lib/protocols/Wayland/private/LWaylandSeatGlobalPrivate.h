#ifndef LWAYLANDSEATGLOBALPRIVATE_H
#define LWAYLANDSEATGLOBALPRIVATE_H

#include <protocols/Wayland/LWaylandSeatGlobal.h>

LPRIVATE_CLASS(LWaylandSeatGlobal)

    // Implementation
    static void bind(wl_client *client, void *compositor, UInt32 version, UInt32 id);
    static void resource_destroy(wl_resource *resource);
    static void get_pointer(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_keyboard(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_touch(wl_client *client, wl_resource *resource, UInt32 id);
#if LOUVRE_SEAT_VERSION >= WL_SEAT_RELEASE_SINCE_VERSION
    static void release(wl_client *client, wl_resource *resource);
#endif

    std::list<LWaylandKeyboardResource*> keyboardResources;
    LastKeyboardEventSerials keyboardSerials;

    std::list<LWaylandPointerResource*> pointerResources;
    LastPointerEventSerials pointerSerials;

    std::list<LWaylandSeatGlobal*>::iterator clientLink;
};

#endif // LWAYLANDSEATGLOBALPRIVATE_H
