#ifndef RREGIONPRIVATE_H
#define RREGIONPRIVATE_H

#include <protocols/Wayland/RRegion.h>
#include <LRegion.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RRegion)
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void add(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void subtract(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);

    LRegion region;
};

#endif // RREGIONPRIVATE_H
