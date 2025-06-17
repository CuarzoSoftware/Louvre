#ifndef GDRMLEASEDEVICE_H
#define GDRMLEASEDEVICE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/CZWeak.h>
#include <CZ/Louvre/LGPU.h>

class Louvre::Protocols::DRMLease::GDRMLeaseDevice final : public LResource
{
public:

    LGPU *gpu() const noexcept
    {
        return m_gpu;
    }

    /******************** REQUESTS ********************/

    static void create_lease_request(wl_client *client, wl_resource *resource, UInt32 id);
    static void release(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    void drmFd() noexcept;
    // Returns false if failed or already created
    bool connector(LOutput *output) noexcept;
    void done() noexcept;
    void released() noexcept;

private:
    friend class Protocols::DRMLease::RDRMLeaseConnector;
    LGLOBAL_INTERFACE
    GDRMLeaseDevice(LGPU *gpu, wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GDRMLeaseDevice() noexcept;
    CZWeak<LGPU> m_gpu;
    std::vector<RDRMLeaseConnector*> m_connectors;
    bool m_roFDSent { false };
};

#endif // GDRMLEASEDEVICE_H
