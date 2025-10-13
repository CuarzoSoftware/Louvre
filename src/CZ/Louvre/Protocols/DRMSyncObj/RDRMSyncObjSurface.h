#ifndef RDRMSYNCOBJSURFACE_H
#define RDRMSYNCOBJSURFACE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::DRMSyncObj::RDRMSyncObjSurface final : public LResource
{
public:
    CZWeak<Protocols::Wayland::RWlSurface> surfaceRes() const noexcept { return m_surfaceRes; }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_acquire_point(wl_client *client, wl_resource *resource, wl_resource *timeline, UInt32 pointHi, UInt32 pointLo);
    static void set_release_point(wl_client *client, wl_resource *resource, wl_resource *timeline, UInt32 pointHi, UInt32 pointLo);
private:
    friend class GDRMSyncObjManager;
    RDRMSyncObjSurface(Protocols::Wayland::RWlSurface *surfaceRes, Int32 version, UInt32 id);
    ~RDRMSyncObjSurface() noexcept = default;
    CZWeak<Protocols::Wayland::RWlSurface> m_surfaceRes;
};

#endif // RDRMSYNCOBJSURFACE_H
