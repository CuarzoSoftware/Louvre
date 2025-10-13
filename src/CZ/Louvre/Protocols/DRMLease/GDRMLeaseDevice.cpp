#include <CZ/Louvre/Protocols/DRMLease/drm-lease-v1.h>
#include <CZ/Louvre/Protocols/DRMLease/GDRMLeaseDevice.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLeaseConnector.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLeaseRequest.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LGlobal.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LOutput.h>

using namespace CZ;
using namespace CZ::Protocols::DRMLease;

static const struct wp_drm_lease_device_v1_interface imp
{
    .create_lease_request = &GDRMLeaseDevice::create_lease_request,
    .release = &GDRMLeaseDevice::release
};

bool GDRMLeaseDevice::Probe(CZWeak<LGlobal> **slot) noexcept
{
    CZ_UNUSED(slot)
    return true;
}

void GDRMLeaseDevice::Bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept
{
    auto &global { *static_cast<LGlobal*>(data) };
    new GDRMLeaseDevice((RDevice*)global.userData, client, version, id);
}

Int32 GDRMLeaseDevice::MaxVersion() noexcept
{
    return LOUVRE_DRM_LEASE_DEVICE_VERSION;
}

const wl_interface *GDRMLeaseDevice::Interface() noexcept
{
    return &wp_drm_lease_device_v1_interface;
}

GDRMLeaseDevice::GDRMLeaseDevice
    (
        RDevice *device,
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
    ),
    m_device(device)
{
    this->client()->imp()->drmLeaseDeviceGlobals.emplace_back(this);
    drmFd();
    for (LOutput *output : seat()->outputs())
        connector(output);
    done();
}

GDRMLeaseDevice::~GDRMLeaseDevice() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->drmLeaseDeviceGlobals, this);
}

/******************** EVENTS ********************/

void GDRMLeaseDevice::drmFd() noexcept
{
    if (m_roFDSent)
        return;

    wp_drm_lease_device_v1_send_drm_fd(resource(), m_device->drmFdReadOnly());
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

void GDRMLeaseDevice::create_lease_request(wl_client */*client*/, wl_resource *resource, UInt32 id)
{
    auto &res { LRES_CAST(GDRMLeaseDevice, resource) };
    new RDRMLeaseRequest(&res, id);
}

void GDRMLeaseDevice::release(wl_client */*client*/, wl_resource *resource)
{
    auto &res { LRES_CAST(GDRMLeaseDevice, resource) };
    res.released();
    res.destroy();
}

