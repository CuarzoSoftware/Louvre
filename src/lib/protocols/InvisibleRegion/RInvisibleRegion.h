#ifndef RINVISIBLEREGION_H
#define RINVISIBLEREGION_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::InvisibleRegion::RInvisibleRegion final : public LResource
{
public:
    Wayland::RSurface *surfaceRes() const noexcept { return m_surfaceRes; }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_region(wl_client *client, wl_resource *resource, wl_resource *region);
private:
    friend class GInvisibleRegionManager;
    RInvisibleRegion(Wayland::RSurface *surfaceRes, UInt32 id, Int32 version) noexcept;
    ~RInvisibleRegion() noexcept;
    LWeak<Wayland::RSurface> m_surfaceRes;
};

#endif // RINVISIBLEREGION_H
