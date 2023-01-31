#include "Region.h"

#include <LRegion.h>
#include <LClient.h>
#include <stdio.h>
#include <LCompositor.h>

void Louvre::Globals::Region::resource_destroy(wl_resource *resource)
{
    LRegion *region = (LRegion*)wl_resource_get_user_data(resource);
    delete region;
}

void Louvre::Globals::Region::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void Louvre::Globals::Region::add(wl_client *, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    LRegion *region = (LRegion*)wl_resource_get_user_data(resource);

    if(width > MAX_SURFACE_SIZE)
        width = MAX_SURFACE_SIZE;
    if(height > MAX_SURFACE_SIZE)
        height = MAX_SURFACE_SIZE;

    region->addRect(LRect(x,y,width,height));
}

void Louvre::Globals::Region::subtract(wl_client *, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    LRegion *region = (LRegion*)wl_resource_get_user_data(resource);

    if(width > MAX_SURFACE_SIZE)
        width = MAX_SURFACE_SIZE;
    if(height > MAX_SURFACE_SIZE)
        height = MAX_SURFACE_SIZE;

    region->subtractRect(LRect(x,y,width,height));
}
