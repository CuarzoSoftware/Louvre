#ifndef LWAYLANDKEYBOARDRESOURCEPRIVATE_H
#define LWAYLANDKEYBOARDRESOURCEPRIVATE_H

#include <protocols/Wayland/LWaylandKeyboardResource.h>

LPRIVATE_CLASS(LWaylandKeyboardResource)

    static void resource_destroy(wl_resource *resource);
#if LOUVRE_SEAT_VERSION >= WL_KEYBOARD_RELEASE_SINCE_VERSION
    static void release(wl_client *client, wl_resource *resource);
#endif

    LWaylandSeatGlobal *seatGlobal;
    std::list<LWaylandKeyboardResource*>::iterator seatLink;
};

#endif // LWAYLANDKEYBOARDRESOURCEPRIVATE_H
