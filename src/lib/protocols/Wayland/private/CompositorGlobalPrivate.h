#ifndef COMPOSITORGLOBALPRIVATE_H
#define COMPOSITORGLOBALPRIVATE_H

#include <protocols/Wayland/CompositorGlobal.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(CompositorGlobal)

    static void resource_destroy(wl_resource *resource);
    static void create_surface(wl_client *client, wl_resource *resource, UInt32 id);
    static void create_region (wl_client *client, wl_resource *resource, UInt32 id);
    static void client_disconect(wl_resource *resource);
};


#endif // COMPOSITORGLOBALPRIVATE_H
