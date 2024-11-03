#include <protocols/DRMLease/drm-lease-v1.h>
#include <protocols/DRMLease/GDRMLeaseDevice.h>
#include <protocols/DRMLease/RDRMLeaseConnector.h>
#include <LUtils.h>
#include <LOutput.h>
#include <cassert>

using namespace Louvre::Protocols::DRMLease;

static const struct wp_drm_lease_connector_v1_interface imp
{
    .destroy = &RDRMLeaseConnector::destroy
};

RDRMLeaseConnector::RDRMLeaseConnector
    (
        GDRMLeaseDevice *drmLeaseDeviceRes,
        LOutput *output
    )
    :LResource
    (
        drmLeaseDeviceRes->client(),
        &wp_drm_lease_connector_v1_interface,
        drmLeaseDeviceRes->version(),
        0,
        &imp
    ),
    m_drmLeaseDeviceRes(drmLeaseDeviceRes),
    m_output(output)
{
    m_drmLeaseDeviceRes->m_connectors.emplace_back(this);
    name();
    description();
    connectorId();
    done();
}

RDRMLeaseConnector::~RDRMLeaseConnector()
{
    if (m_drmLeaseDeviceRes)
        LVectorRemoveOneUnordered(m_drmLeaseDeviceRes->m_connectors, this);
}

/******************** REQUESTS ********************/

void RDRMLeaseConnector::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RDRMLeaseConnector::name() noexcept
{
    assert(m_output != nullptr);
    wp_drm_lease_connector_v1_send_name(resource(), m_output->name());
}

void RDRMLeaseConnector::description() noexcept
{
    assert(m_output != nullptr);
    wp_drm_lease_connector_v1_send_description(resource(), m_output->description());
}

void RDRMLeaseConnector::connectorId() noexcept
{
    assert(m_output != nullptr);
    wp_drm_lease_connector_v1_send_connector_id(resource(), m_output->id());
}

void RDRMLeaseConnector::done() noexcept
{
    wp_drm_lease_connector_v1_send_done(resource());
}

void RDRMLeaseConnector::withdrawn() noexcept
{
    wp_drm_lease_connector_v1_send_withdrawn(resource());
}
