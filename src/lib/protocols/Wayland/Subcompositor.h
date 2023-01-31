#ifndef SUBCOMPOSITOR_H
#define SUBCOMPOSITOR_H

#include <LNamespaces.h>

class Louvre::Globals::Subcompositor
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_subsurface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *parent);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
};

#endif // SUBCOMPOSITOR_H
