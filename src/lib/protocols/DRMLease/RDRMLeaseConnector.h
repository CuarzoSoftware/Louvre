#ifndef RDRMLEASECONNECTOR_H
#define RDRMLEASECONNECTOR_H

#include <LResource.h>
#include <LWeak.h>
#include <string>

class Louvre::Protocols::DRMLease::RDRMLeaseConnector final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    void name() noexcept;
    void description() noexcept;
    void connectorId() noexcept;
    void done() noexcept;
    void withdrawn() noexcept;

private:
    friend class GDRMLeaseDevice;
    RDRMLeaseConnector(GDRMLeaseDevice *leaseDeviceRes, LOutput *output);
    ~RDRMLeaseConnector();
    LWeak<GDRMLeaseDevice> m_drmLeaseDeviceRes;
    LWeak<LOutput> m_output;
};


#endif // RDRMLEASECONNECTOR_H
