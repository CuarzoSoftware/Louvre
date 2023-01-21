#ifdef LOUVRE_DMA_ENABLE
#ifndef LINUXDMABUF_H
#define LINUXDMABUF_H

#include <LNamespaces.h>

class Louvre::Extensions::LinuxDMABuf::LinuxDMABuf
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void create_params(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_default_feedback(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_surface_feedback(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
};

#endif // LINUXDMABUF_H
#endif
