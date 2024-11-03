#include <protocols/DRMLease/drm-lease-v1.h>
#include <protocols/DRMLease/GDRMLeaseDevice.h>
#include <protocols/DRMLease/RDRMLeaseConnector.h>
#include <private/LClientPrivate.h>
#include <LGlobal.h>
#include <LUtils.h>
#include <LSeat.h>
#include <LOutput.h>

using namespace Louvre;
using namespace Louvre::Protocols::DRMLease;

static const struct wp_drm_lease_device_v1_interface imp
{
    .create_lease_request = &GDRMLeaseDevice::create_lease_request,
    .release = &GDRMLeaseDevice::release
};

void GDRMLeaseDevice::bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept
{
    auto &global { *static_cast<LGlobal*>(data) };
    new GDRMLeaseDevice((LGPU*)global.userData(), client, version, id);
}

Int32 GDRMLeaseDevice::maxVersion() noexcept
{
    return LOUVRE_DRM_LEASE_DEVICE_VERSION;
}

const wl_interface *GDRMLeaseDevice::interface() noexcept
{
    return &wp_drm_lease_device_v1_interface;
}

GDRMLeaseDevice::GDRMLeaseDevice
    (
        LGPU *gpu,
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
    ),
    m_gpu(gpu)
{
    this->client()->imp()->drmLeaseDeviceGlobals.emplace_back(this);
    drmFd();
    for (LOutput *output : seat()->outputs())
        connector(output);
    done();
}

GDRMLeaseDevice::~GDRMLeaseDevice() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->drmLeaseDeviceGlobals, this);
}

/******************** EVENTS ********************/

void GDRMLeaseDevice::drmFd() noexcept
{
    if (m_roFDSent || !m_gpu || m_gpu->roFd() != -1)
        return;

    wp_drm_lease_device_v1_send_drm_fd(resource(), m_gpu->roFd());
    m_roFDSent = true;
}

bool GDRMLeaseDevice::connector(LOutput *output) noexcept
{
    if (!output->leasable())
        return false;

    for (RDRMLeaseConnector *conn : m_connectors)
        if (conn->m_output == output)
            return false;

    new RDRMLeaseConnector(this, output);
    return true;
}

void GDRMLeaseDevice::released() noexcept
{
    wp_drm_lease_device_v1_send_released(resource());
}

void GDRMLeaseDevice::done() noexcept
{
    wp_drm_lease_device_v1_send_done(resource());
}

/******************** REQUESTS ********************/

void GDRMLeaseDevice::create_lease_request(wl_client *client, wl_resource *resource, UInt32 id)
{

}

void GDRMLeaseDevice::release(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<GDRMLeaseDevice*>(wl_resource_get_user_data(resource)) };
    res.released();
    res.destroy();
}

