#include <protocols/Wayland/private/RRegionPrivate.h>
#include <pixman.h>

void RRegion::RRegionPrivate::resource_destroy(wl_resource *resource)
{
    RRegion *rRegion = (RRegion*)wl_resource_get_user_data(resource);
    delete rRegion;
}

void RRegion::RRegionPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RRegion::RRegionPrivate::add(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    RRegion *rRegion = (RRegion*)wl_resource_get_user_data(resource);

    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;
    else if (width <= 0)
        return;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;
    else if (height <= 0)
        return;

    pixman_region32_union_rect(&rRegion->imp()->added.m_region,
                               &rRegion->imp()->added.m_region,
                               x, y, width, height);
}

void RRegion::RRegionPrivate::subtract(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    RRegion *rRegion = (RRegion*)wl_resource_get_user_data(resource);

    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;
    else if (width <= 0)
        return;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;
    else if (height <= 0)
        return;

    pixman_region32_union_rect(&rRegion->imp()->pendingSubtract.m_region,
                               &rRegion->imp()->pendingSubtract.m_region,
                               x, y, width, height);
}
