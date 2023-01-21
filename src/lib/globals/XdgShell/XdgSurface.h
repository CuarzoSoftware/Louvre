#ifndef XDGSURFACE_H
#define XDGSURFACE_H

#include <LNamespaces.h>

class Louvre::Extensions::XdgShell::Surface
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_toplevel(wl_client *client,wl_resource *resource, UInt32 id);
    static void get_popup(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *parent, wl_resource *positioner);
    static void set_window_geometry(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void ack_configure(wl_client *client, wl_resource *resource, UInt32 serial);
};

#endif // XDGSURFACE_H
