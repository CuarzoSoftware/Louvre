#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/linux-drm-syncobj-v1.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/GDRMSyncObjManager.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/RDRMSyncObjTimeline.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/RDRMSyncObjSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Ream/DRM/RDRMTimeline.h>
#include <CZ/Ream/RDevice.h>

using namespace CZ;
using namespace CZ::Protocols::DRMSyncObj;

static const struct wp_linux_drm_syncobj_manager_v1_interface imp
{
    .destroy = &GDRMSyncObjManager::destroy,
    .get_surface = &GDRMSyncObjManager::get_surface,
    .import_timeline = &GDRMSyncObjManager::import_timeline,
};

LGLOBAL_INTERFACE_IMP(GDRMSyncObjManager, LOUVRE_DRM_SYNC_OBJ_MANAGER_VERSION, wp_linux_drm_syncobj_manager_v1_interface)

bool GDRMSyncObjManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    auto ream { RCore::Get() };
    auto *mainDev { ream->mainDevice() };

    if (!mainDev->caps().Timeline)
    {
        LLog(CZWarning, CZLN, "Failed to create {} global (DRM timelines not supported by the main Ream device)", Interface()->name);
        return false;
    }

    if (compositor()->wellKnownGlobals.DRMSyncObjManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.DRMSyncObjManager;
    return true;
}

GDRMSyncObjManager::GDRMSyncObjManager
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
    this->client()->imp()->drmSyncObjManagerGlobals.emplace_back(this);
}

GDRMSyncObjManager::~GDRMSyncObjManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->drmSyncObjManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GDRMSyncObjManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GDRMSyncObjManager::get_surface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    auto &res { *static_cast<GDRMSyncObjManager*>(wl_resource_get_user_data(resource)) };
    auto &surfaceRes { *static_cast<Protocols::Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes.surface()->imp()->pending.drmSyncObjSurfaceRes)
    {
        res.postError(WP_LINUX_DRM_SYNCOBJ_MANAGER_V1_ERROR_SURFACE_EXISTS, "The surface already has a drm syncobj resource");
        return;
    }

    // TODO: Handle out of memory
    new RDRMSyncObjSurface(&surfaceRes, res.version(), id);
}

void GDRMSyncObjManager::import_timeline(wl_client */*client*/, wl_resource *resource, UInt32 id, Int32 fd) noexcept
{
    auto &res { *static_cast<GDRMSyncObjManager*>(wl_resource_get_user_data(resource)) };
    auto timeline { RDRMTimeline::Import(fd, CZOwn::Own) };

    if (!timeline)
    {
        res.postError(WP_LINUX_DRM_SYNCOBJ_MANAGER_V1_ERROR_INVALID_TIMELINE, "Failed to import timeline");
        return;
    }

    // TODO: Handle out of memory
    new RDRMSyncObjTimeline(std::move(timeline), res.client(), res.version(), id);
}

