#include <CZ/Louvre/Protocols/DRMLease/drm-lease-v1.h>
#include <CZ/Louvre/Protocols/DRMLease/GDRMLeaseDevice.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLeaseConnector.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <cassert>

using namespace CZ::Protocols::DRMLease;

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
    wp_drm_lease_device_v1_send_connector(m_drmLeaseDeviceRes->resource(), resource());
    m_drmLeaseDeviceRes->m_connectors.emplace_back(this);
    m_output->imp()->drmLeaseConnectorRes.emplace_back(this);
    name();
    description();
    connectorId();
    done();
}

RDRMLeaseConnector::~RDRMLeaseConnector()
{
    if (m_drmLeaseDeviceRes)
        CZVectorUtils::RemoveOneUnordered(m_drmLeaseDeviceRes->m_connectors, this);

    if (m_output)
        CZVectorUtils::RemoveOneUnordered(m_output->imp()->drmLeaseConnectorRes, this);
}

/******************** REQUESTS ********************/

void RDRMLeaseConnector::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RDRMLeaseConnector::name() noexcept
{
    assert(m_output.get() != nullptr);
    wp_drm_lease_connector_v1_send_name(resource(), m_output->name().c_str());
}

void RDRMLeaseConnector::description() noexcept
{
    assert(m_output.get() != nullptr);
    wp_drm_lease_connector_v1_send_description(resource(), m_output->description().c_str());
}

void RDRMLeaseConnector::connectorId() noexcept
{
    assert(m_output.get() != nullptr);
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
