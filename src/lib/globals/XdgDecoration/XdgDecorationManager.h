#ifndef XDGDECORATIONMANAGER_H
#define XDGDECORATIONMANAGER_H

#include <LNamespaces.h>

class Louvre::Extensions::XdgDecoration::Manager
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_toplevel_decoration(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *toplevel);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
};

#endif // XDGDECORATIONMANAGER_H
