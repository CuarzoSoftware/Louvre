#ifndef RDRMLEASEREQUEST_H
#define RDRMLEASEREQUEST_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Louvre/LOutput.h>
#include <CZ/Louvre/LGPU.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::DRMLease::RDRMLeaseRequest final : public LResource
{
public:

    LGPU *gpu() const noexcept
    {
        return m_gpu;
    }

    const std::vector<CZWeak<LOutput>> &requestedConnectors() const noexcept
    {
        return m_requestedConnectors;
    }

    /******************** REQUESTS ********************/

    static void request_connector(wl_client *client, wl_resource *resource, wl_resource *connector);
    static void submit(wl_client *client, wl_resource *resource, UInt32 id);

private:
    friend class GDRMLeaseDevice;
    RDRMLeaseRequest(GDRMLeaseDevice *leaseDeviceRes, UInt32 id);
    ~RDRMLeaseRequest() = default;
    CZWeak<LGPU> m_gpu;
    std::vector<CZWeak<LOutput>> m_requestedConnectors;
    bool m_addedConnector { false };
};

#endif // RDRMLEASEREQUEST_H
