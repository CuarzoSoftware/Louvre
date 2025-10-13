#include <CZ/Louvre/Protocols/DRMSyncObj/RDRMSyncObjSurface.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/RDRMSyncObjTimeline.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/linux-drm-syncobj-v1.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::DRMSyncObj;

static const struct wp_linux_drm_syncobj_surface_v1_interface imp
{
    .destroy = &RDRMSyncObjSurface::destroy,
    .set_acquire_point = &RDRMSyncObjSurface::set_acquire_point,
    .set_release_point = &RDRMSyncObjSurface::set_release_point,
};

RDRMSyncObjSurface::RDRMSyncObjSurface
    (
        Protocols::Wayland::RWlSurface *surfaceRes,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        surfaceRes->client(),
        &wp_linux_drm_syncobj_surface_v1_interface,
        version,
        id,
        &imp
    ),
    m_surfaceRes(surfaceRes)
{
    surfaceRes->surface()->imp()->pending.drmSyncObjSurfaceRes.reset(this);
}

/******************** REQUESTS ********************/

void RDRMSyncObjSurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RDRMSyncObjSurface::set_acquire_point(wl_client */*client*/, wl_resource *resource, wl_resource *timeline, UInt32 pointHi, UInt32 pointLo)
{
    auto &res { *static_cast<RDRMSyncObjSurface*>(wl_resource_get_user_data(resource)) };
    auto &tl { *static_cast<RDRMSyncObjTimeline*>(wl_resource_get_user_data(timeline)) };

    if (!res.surfaceRes())
    {
        res.postError(WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_SURFACE, "Missing wl_surface");
        return;
    }

    res.surfaceRes()->surface()->imp()->pending.buffer.acquirePoint = static_cast<UInt64>(pointHi) << 32 | pointLo;
    res.surfaceRes()->surface()->imp()->pending.buffer.acquireTimeline = tl.timeline();
}

void RDRMSyncObjSurface::set_release_point(wl_client */*client*/, wl_resource *resource, wl_resource *timeline, UInt32 pointHi, UInt32 pointLo)
{
    auto &res { *static_cast<RDRMSyncObjSurface*>(wl_resource_get_user_data(resource)) };
    auto &tl { *static_cast<RDRMSyncObjTimeline*>(wl_resource_get_user_data(timeline)) };

    if (!res.surfaceRes())
    {
        res.postError(WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_SURFACE, "Missing wl_surface");
        return;
    }

    res.surfaceRes()->surface()->imp()->pending.buffer.releasePoint = static_cast<UInt64>(pointHi) << 32 | pointLo;
    res.surfaceRes()->surface()->imp()->pending.buffer.releaseTimeline = tl.timeline();
}
