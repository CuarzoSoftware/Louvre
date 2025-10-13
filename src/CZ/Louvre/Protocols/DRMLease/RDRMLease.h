#ifndef RDRMLEASE_H
#define RDRMLEASE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/SRM/SRMLease.h>

class CZ::Protocols::DRMLease::RDRMLease final : public LResource
{
public:

    /* May not exactly match the request, depends on all LOutput::leaseRequest() answers */
    const std::unordered_set<LOutput*> &leasedConnectors() const noexcept
    {
        return m_leasedConnectors;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    void finished();

private:
    friend class RDRMLeaseRequest;
    RDRMLease(RDRMLeaseRequest *leaseRequestRes, UInt32 id);
    ~RDRMLease();
    CZWeak<RDevice> m_device;
    std::unordered_set<LOutput*> m_leasedConnectors;
    std::shared_ptr<SRMLease> m_lease;
    bool m_finished { false };

    /******************** PRIVATE EVENTS ********************/

    void sendFd(int fd);
};


#endif // RDRMLEASE_H
