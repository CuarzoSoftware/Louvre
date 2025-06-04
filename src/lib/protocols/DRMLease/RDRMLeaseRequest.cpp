#include <protocols/DRMLease/drm-lease-v1.h>
#include <protocols/DRMLease/GDRMLeaseDevice.h>
#include <protocols/DRMLease/RDRMLeaseRequest.h>
#include <protocols/DRMLease/RDRMLeaseConnector.h>
#include <protocols/DRMLease/RDRMLease.h>
#include <LUtils.h>
#include <LOutput.h>

using namespace Louvre::Protocols::DRMLease;

static const struct wp_drm_lease_request_v1_interface imp
{
    .request_connector = &RDRMLeaseRequest::request_connector,
    .submit = &RDRMLeaseRequest::submit
};

RDRMLeaseRequest::RDRMLeaseRequest
    (
        GDRMLeaseDevice *drmLeaseDeviceRes,
        UInt32 id
    )
    :LResource
    (
        drmLeaseDeviceRes->client(),
        &wp_drm_lease_request_v1_interface,
        drmLeaseDeviceRes->version(),
        id,
        &imp
    ),
    m_gpu(drmLeaseDeviceRes->gpu())
{}

/******************** REQUESTS ********************/

void RDRMLeaseRequest::request_connector(wl_client */*client*/, wl_resource *resource, wl_resource *connector)
{
    auto &res { LRES_CAST(RDRMLeaseRequest, resource) };
    LOutput *output { LRES_CAST(RDRMLeaseConnector, connector).output() };

    // Maybe the output was destroyed, not the client's fault
    if (!output)
    {
        res.m_addedConnector = true;
        return;
    }

    if (output->gpu() != res.gpu())
    {
        res.postError(WP_DRM_LEASE_REQUEST_V1_ERROR_WRONG_DEVICE, "Requested a connector from a different lease device.");
        return;
    }

    for (auto &conn : res.requestedConnectors())
    {
        if (conn.get() == output)
        {
            res.postError(WP_DRM_LEASE_REQUEST_V1_ERROR_DUPLICATE_CONNECTOR, "Requested a connector twice.");
            return;
        }
    }

    res.m_addedConnector = true;
    res.m_requestedConnectors.emplace_back(output);
}

void RDRMLeaseRequest::submit(wl_client */*client*/, wl_resource *resource, UInt32 id)
{
    auto &res { LRES_CAST(RDRMLeaseRequest, resource) };

    if (!res.m_addedConnector)
    {
        res.postError(WP_DRM_LEASE_REQUEST_V1_ERROR_EMPTY_LEASE, "Requested a lease without requesting a connector.");
        return;
    }

    new RDRMLease(&res, id);
    res.destroy();
}
