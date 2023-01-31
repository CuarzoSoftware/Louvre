#ifndef LWAYLANDPOINTERRESOURCEPRIVATE_H
#define LWAYLANDPOINTERRESOURCEPRIVATE_H

#include <protocols/Wayland/LWaylandPointerResource.h>

LPRIVATE_CLASS(LWaylandPointerResource)

    static void resource_destroy(wl_resource *resource);
    static void set_cursor(wl_client *client, wl_resource *resource, UInt32 serial, wl_resource *_surface, Int32 hotspot_x, Int32 hotspot_y);
#if LOUVRE_SEAT_VERSION >= WL_SEAT_RELEASE_SINCE_VERSION
    static void release(wl_client *client, wl_resource *resource);
#endif

    LWaylandSeatGlobal *seatGlobal;
    std::list<LWaylandPointerResource*>::iterator seatLink;
};

#endif // LWAYLANDPOINTERRESOURCEPRIVATE_H
