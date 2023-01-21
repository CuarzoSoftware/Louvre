#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <LNamespaces.h>

class Louvre::Globals::Compositor
{
public:
    static void resource_destroy(wl_resource *resource);
    static void create_surface(wl_client *client, wl_resource *resource, UInt32 id);
    static void create_region (wl_client *client, wl_resource *resource, UInt32 id);
    static void client_disconect(wl_resource *resource);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
};

#endif // COMPOSITOR_H
