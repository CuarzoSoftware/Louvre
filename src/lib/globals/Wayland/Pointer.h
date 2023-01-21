#ifndef POINTER_H
#define POINTER_H

#include <LNamespaces.h>

class Louvre::Globals::Pointer
{
public:
    static void resource_destroy(wl_resource *resource);
    static void set_cursor(wl_client *client, wl_resource *resource, UInt32 serial, wl_resource *_surface, Int32 hotspot_x, Int32 hotspot_y);
#if LOUVRE_SEAT_VERSION >= 3
    static void release(wl_client *client, wl_resource *resource);
#endif
};
#endif // POINTER_H
