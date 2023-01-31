#ifndef XDGWMBASE_H
#define XDGWMBASE_H

#include <LNamespaces.h>

class Louvre::Extensions::XdgShell::WmBase
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void create_positioner(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_xdg_surface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);
    static void pong(wl_client *client, wl_resource *resource, UInt32 serial);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
};

#endif // XDGWMBASE_H
