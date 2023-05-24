#ifdef nn

#ifndef LINUXDMABUFFER_H
#define LINUXDMABUFFER_H

#include <LResource.h>

class Louvre::Protocols::LinuxDMABuff::LinuxDMABuff : public LResource
{
public:
    GCompositor(LClient *client,
                     const wl_interface *interface,
                     Int32 version,
                     UInt32 id,
                     const void *implementation,
                     wl_resource_destroy_func_t destroy);

    ~GCompositor();

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);

    LPRIVATE_IMP(GCompositor)
};

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
#endif
