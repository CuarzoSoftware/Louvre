#include <CZ/Louvre/Protocols/InvisibleRegion/GInvisibleRegionManager.h>
#include <CZ/Louvre/Protocols/InvisibleRegion/RInvisibleRegion.h>
#include <CZ/Louvre/Protocols/InvisibleRegion/lvr-invisible-region.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::InvisibleRegion;

static const struct lvr_invisible_region_manager_interface imp
{
    .destroy = &GInvisibleRegionManager::destroy,
    .get_invisible_region = &GInvisibleRegionManager::get_invisible_region
};

void GInvisibleRegionManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GInvisibleRegionManager(client, version, id);
}

Int32 GInvisibleRegionManager::maxVersion() noexcept
{
    return LOUVRE_INVISIBLE_REGION_MANAGER_VERSION;
}

const wl_interface *GInvisibleRegionManager::interface() noexcept
{
    return &lvr_invisible_region_manager_interface;
}

GInvisibleRegionManager::GInvisibleRegionManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->invisibleRegionManagerGlobals.emplace_back(this);
}

GInvisibleRegionManager::~GInvisibleRegionManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->invisibleRegionManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GInvisibleRegionManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GInvisibleRegionManager::get_invisible_region(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    auto *surfaceRes { static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->surface()->imp()->invisibleRegion)
    {
        surfaceRes->postError(
            LVR_INVISIBLE_REGION_MANAGER_ERROR_ALREADY_CONSTRUCTED,
            "the surface already has an associated lvr_invisible_region object");
        return;
    }

    new RInvisibleRegion(
        surfaceRes,
        id,
        wl_resource_get_version(resource));
}
