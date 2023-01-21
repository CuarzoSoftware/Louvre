#ifndef SEAT_H
#define SEAT_H

#include <LNamespaces.h>

class Louvre::Globals::Seat
{
public:
    static void resource_destroy(wl_resource *resource);
    static void get_pointer(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_keyboard(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_touch(wl_client *client, wl_resource *resource, UInt32 id);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);

#if LOUVRE_SEAT_VERSION >= 5
    static void release(wl_client *client, wl_resource *resource);
#endif

};

#endif // SEAT_H
