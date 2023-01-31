#ifndef LINUXDMABUFFER_H
#define LINUXDMABUFFER_H

#include <LNamespaces.h>

class Louvre::Extensions::LinuxDMABuffer::LinuxDMABuffer
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void create_params(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_default_feedback(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_surface_feedback(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
};

#endif // LINUXDMABUFFER_H
