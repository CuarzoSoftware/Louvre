#ifndef RINVISIBLEREGION_H
#define RINVISIBLEREGION_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::InvisibleRegion::RInvisibleRegion final : public LResource
{
public:
    Wayland::RWlSurface *surfaceRes() const noexcept { return m_surfaceRes; }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_region(wl_client *client, wl_resource *resource, wl_resource *region);
private:
    friend class GInvisibleRegionManager;
    RInvisibleRegion(Wayland::RWlSurface *surfaceRes, UInt32 id, Int32 version) noexcept;
    ~RInvisibleRegion() noexcept;
    CZWeak<Wayland::RWlSurface> m_surfaceRes;
};

#endif // RINVISIBLEREGION_H
