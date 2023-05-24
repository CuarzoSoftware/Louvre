#include <protocols/Wayland/private/RRegionPrivate.h>

void RRegion::RRegionPrivate::resource_destroy(wl_resource *resource)
{
    RRegion *lRRegion = (RRegion*)wl_resource_get_user_data(resource);
    delete lRRegion;
}

void RRegion::RRegionPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    wl_resource_destroy(resource);
}

void RRegion::RRegionPrivate::add(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    RRegion *lRRegion = (RRegion*)wl_resource_get_user_data(resource);

    if (width > MAX_SURFACE_SIZE)
        width = MAX_SURFACE_SIZE;
    if (height > MAX_SURFACE_SIZE)
        height = MAX_SURFACE_SIZE;

    lRRegion->imp()->region.addRect(LRect(x,y,width,height));
}

void RRegion::RRegionPrivate::subtract(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    RRegion *lRRegion = (RRegion*)wl_resource_get_user_data(resource);

    if (width > MAX_SURFACE_SIZE)
        width = MAX_SURFACE_SIZE;
    if (height > MAX_SURFACE_SIZE)
        height = MAX_SURFACE_SIZE;

    lRRegion->imp()->region.subtractRect(LRect(x,y,width,height));
}
