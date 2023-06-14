#ifndef GCOMPOSITORPRIVATE_H
#define GCOMPOSITORPRIVATE_H

#include <protocols/Wayland/GCompositor.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(GCompositor)
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
    static void resource_destroy(wl_resource *resource);
    static void create_surface(wl_client *client, wl_resource *resource, UInt32 id);
    static void create_region (wl_client *client, wl_resource *resource, UInt32 id);
    std::list<GCompositor*>::iterator clientLink;
};

#endif // GCOMPOSITORPRIVATE_H
