#ifndef RDRMLEASE_H
#define RDRMLEASE_H

#include <LResource.h>
#include <LOutput.h>
#include <LGPU.h>
#include <LWeak.h>

class Louvre::Protocols::DRMLease::RDRMLease final : public LResource
{
public:

    /* May not exactly match the request, depends on all LOutput::leaseRequest() answers */
    const std::vector<LOutput*> &leasedConnectors() const noexcept
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
    LWeak<LGPU> m_gpu;
    std::vector<LOutput*> m_leasedConnectors;
    int m_fd { -1 };
    bool m_finished { false };

    /******************** PRIVATE EVENTS ********************/

    void sendFd(int fd);
};


#endif // RDRMLEASE_H
