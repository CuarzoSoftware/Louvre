#ifndef REGIONRESOURCEPRIVATE_H
#define REGIONRESOURCEPRIVATE_H

#include <LRegion.h>
#include <protocols/Wayland/RegionResource.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RegionResource)
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void add(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void subtract(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);

    LRegion region;
};


#endif // REGIONRESOURCEPRIVATE_H
