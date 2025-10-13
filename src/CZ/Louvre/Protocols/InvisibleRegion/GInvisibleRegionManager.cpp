#include <CZ/Louvre/Protocols/InvisibleRegion/GInvisibleRegionManager.h>
#include <CZ/Louvre/Protocols/InvisibleRegion/RInvisibleRegion.h>
#include <CZ/Louvre/Protocols/InvisibleRegion/lvr-invisible-region.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::InvisibleRegion;

static const struct lvr_invisible_region_manager_interface imp
{
    .destroy = &GInvisibleRegionManager::destroy,
    .get_invisible_region = &GInvisibleRegionManager::get_invisible_region
};

LGLOBAL_INTERFACE_IMP(GInvisibleRegionManager, LOUVRE_INVISIBLE_REGION_MANAGER_VERSION, lvr_invisible_region_manager_interface)

bool GInvisibleRegionManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.LvrInvisibleRegionManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.LvrInvisibleRegionManager;
    return true;
}

GInvisibleRegionManager::GInvisibleRegionManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->invisibleRegionManagerGlobals.emplace_back(this);
}

GInvisibleRegionManager::~GInvisibleRegionManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->invisibleRegionManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GInvisibleRegionManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GInvisibleRegionManager::get_invisible_region(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

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
