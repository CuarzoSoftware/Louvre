#include <protocols/InvisibleRegion/RInvisibleRegion.h>
#include <protocols/InvisibleRegion/lvr-invisible-region.h>
#include <protocols/Wayland/RRegion.h>
#include <private/LSurfacePrivate.h>

using namespace Louvre::Protocols::InvisibleRegion;

static const struct lvr_invisible_region_interface imp
{
    .destroy = &RInvisibleRegion::destroy,
    .set_region = &RInvisibleRegion::set_region,
};

RInvisibleRegion::RInvisibleRegion
    (
        Wayland::RSurface *surface,
        UInt32 id,
        Int32 version
    ) noexcept
    :LResource
    (
        surface->client(),
        &lvr_invisible_region_interface,
        version,
        id,
        &imp
    ), m_surfaceRes(surface)
{}

RInvisibleRegion::~RInvisibleRegion() noexcept
{
    if (!surfaceRes())
    {
        wl_resource_post_error(resource(),
            LVR_INVISIBLE_REGION_ERROR_DESTROYED_SURFACE,
            "surface destroyed before its lvr_invisible_region object");
        return;
    }

    surfaceRes()->surface()->imp()->pendingInvisibleRegion.clear();
    surfaceRes()->surface()->imp()->stateFlags.remove(LSurface::LSurfacePrivate::InfiniteInvisible);
    surfaceRes()->surface()->imp()->changesToNotify.add(LSurface::LSurfacePrivate::InvisibleRegionChanged);
}

/******************** REQUESTS ********************/

void RInvisibleRegion::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RInvisibleRegion::set_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &res { *static_cast<RInvisibleRegion*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource,
                               LVR_INVISIBLE_REGION_ERROR_DESTROYED_SURFACE,
                               "surface destroyed before its lvr_invisible_region object");
        return;
    }

    auto &imp { *res.surfaceRes()->surface()->imp() };

    if (region)
    {
        auto &regionRes { *static_cast<Wayland::RRegion*>(wl_resource_get_user_data(region)) };
        imp.pendingInvisibleRegion = regionRes.region();
        imp.stateFlags.remove(LSurface::LSurfacePrivate::InfiniteInvisible);
        imp.changesToNotify.add(LSurface::LSurfacePrivate::InvisibleRegionChanged);
    }
    else if (!imp.stateFlags.check(LSurface::LSurfacePrivate::InfiniteInvisible))
    {
        imp.stateFlags.add(LSurface::LSurfacePrivate::InfiniteInvisible);
        imp.changesToNotify.add(LSurface::LSurfacePrivate::InvisibleRegionChanged);
    }
}
