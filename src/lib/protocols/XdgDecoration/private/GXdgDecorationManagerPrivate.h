#ifndef GXDGDECORATIONMANAGERPRIVATE_H
#define GXDGDECORATIONMANAGERPRIVATE_H

#include <protocols/XdgDecoration/GXdgDecorationManager.h>
#include <protocols/XdgDecoration/xdg-decoration-unstable-v1.h>

using namespace Louvre::Protocols::XdgDecoration;

LPRIVATE_CLASS(GXdgDecorationManager)
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_toplevel_decoration(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *toplevel);
};

#endif // GXDGDECORATIONMANAGERPRIVATE_H
