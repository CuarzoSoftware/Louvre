#ifndef GSUBCOMPOSITORPRIVATE_H
#define GSUBCOMPOSITORPRIVATE_H

#include <protocols/Wayland/GSubcompositor.h>

using namespace Louvre::Protocols::Wayland;
using namespace std;

LPRIVATE_CLASS(GSubcompositor)
    static void bind(wl_client *client, void *compositor, UInt32 version, UInt32 id);
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_subsurface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *parent);
    list<GSubcompositor*>::iterator clientLink;
};

#endif // GSUBCOMPOSITORPRIVATE_H