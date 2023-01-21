#ifndef SUBSURFACE_H
#define SUBSURFACE_H

#include <LNamespaces.h>

class Louvre::Globals::Subsurface
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void set_position(wl_client *client, wl_resource *resource, Int32 x, Int32 y);
    static void place_above(wl_client *client, wl_resource *resource, wl_resource *sibiling);
    static void place_below(wl_client *client, wl_resource *resource, wl_resource *sibiling);
    static void set_sync(wl_client *client, wl_resource *resource);
    static void set_desync(wl_client *client, wl_resource *resource);

    static void sync_all_children_surfaces(LSurface *surface);
};

#endif // SUBSURFACE_H
