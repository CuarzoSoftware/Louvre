#include <protocols/Wayland/private/RegionResourcePrivate.h>

void RegionResource::RegionResourcePrivate::resource_destroy(wl_resource *resource)
{
    RegionResource *lRegionResource = (RegionResource*)wl_resource_get_user_data(resource);
    delete lRegionResource;
}

void RegionResource::RegionResourcePrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    wl_resource_destroy(resource);
}

void RegionResource::RegionResourcePrivate::add(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    RegionResource *lRegionResource = (RegionResource*)wl_resource_get_user_data(resource);

    if(width > MAX_SURFACE_SIZE)
        width = MAX_SURFACE_SIZE;
    if(height > MAX_SURFACE_SIZE)
        height = MAX_SURFACE_SIZE;

    lRegionResource->imp()->region.addRect(LRect(x,y,width,height));
}

void RegionResource::RegionResourcePrivate::subtract(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    RegionResource *lRegionResource = (RegionResource*)wl_resource_get_user_data(resource);

    if(width > MAX_SURFACE_SIZE)
        width = MAX_SURFACE_SIZE;
    if(height > MAX_SURFACE_SIZE)
        height = MAX_SURFACE_SIZE;

    lRegionResource->imp()->region.subtractRect(LRect(x,y,width,height));
}
