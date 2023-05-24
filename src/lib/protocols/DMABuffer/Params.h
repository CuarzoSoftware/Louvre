#ifdef nn

#ifndef PARAMS_H
#define PARAMS_H

#include <LNamespaces.h>

class Louvre::Extensions::LinuxDMABuffer::Params
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void add(wl_client *client, wl_resource *resource, int fd, UInt32 plane_idx, UInt32 offset, UInt32 stride, UInt32 modifier_hi, UInt32 modifier_lo);
    static void create(wl_client *client, wl_resource *resource, Int32 width, Int32 height, UInt32 format, UInt32 flags);
    static void create_immed(wl_client *client, wl_resource *resource, UInt32 buffer_id, Int32 width, Int32 height, UInt32 format, UInt32 flags);

};

#endif // PARAMS_H
#endif
