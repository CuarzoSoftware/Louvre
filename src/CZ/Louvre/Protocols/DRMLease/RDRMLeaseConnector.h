#ifndef RDRMLEASECONNECTOR_H
#define RDRMLEASECONNECTOR_H

#include <LResource.h>
#include <CZ/CZWeak.h>

class Louvre::Protocols::DRMLease::RDRMLeaseConnector final : public LResource
{
public:

    GDRMLeaseDevice *drmLeaseDeviceRes() const noexcept
    {
        return m_drmLeaseDeviceRes;
    }

    LOutput *output() const noexcept
    {
        return m_output;
    }

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
    CZWeak<GDRMLeaseDevice> m_drmLeaseDeviceRes;
    CZWeak<LOutput> m_output;
};

#endif // RDRMLEASECONNECTOR_H
