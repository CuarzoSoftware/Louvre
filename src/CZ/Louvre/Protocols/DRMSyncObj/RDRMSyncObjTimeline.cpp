#include <CZ/Louvre/Protocols/DRMSyncObj/RDRMSyncObjTimeline.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/linux-drm-syncobj-v1.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>

using namespace CZ::Protocols::DRMSyncObj;

static const struct wp_linux_drm_syncobj_timeline_v1_interface imp
{
    .destroy = &RDRMSyncObjTimeline::destroy
};

RDRMSyncObjTimeline::RDRMSyncObjTimeline
    (
        std::shared_ptr<RDRMTimeline> &&timeline,
        LClient *client,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        client,
        &wp_linux_drm_syncobj_timeline_v1_interface,
        version,
        id,
        &imp
    ),
    m_timeline(std::move(timeline))
{}

/******************** REQUESTS ********************/

void RDRMSyncObjTimeline::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}
