#ifndef RDRMLEASEREQUEST_H
#define RDRMLEASEREQUEST_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::DRMLease::RDRMLeaseRequest final : public LResource
{
public:

    RDevice *device() const noexcept
    {
        return m_device;
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
    CZWeak<RDevice> m_device;
    std::vector<CZWeak<LOutput>> m_requestedConnectors;
    bool m_addedConnector { false };
};

#endif // RDRMLEASEREQUEST_H
