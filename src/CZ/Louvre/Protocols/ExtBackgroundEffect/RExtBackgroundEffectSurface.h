#ifndef REXTBACKGROUNDEFFECTSURFACE_H
#define REXTBACKGROUNDEFFECTSURFACE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

namespace CZ::Protocols::Wayland { class RWlSurface; }

/**
 * @brief ext_background_effect_surface_v1 resource.
 *
 * Writes the requested blur region directly into the surface's
 * LBackgroundBlur with no configure/acknowledge handshake: state
 * Enabled while a non-null region is pending, Disabled once the client
 * sends a null region (removes the effect) or destroys the object.
 */
class CZ::Protocols::ExtBackgroundEffect::RExtBackgroundEffectSurface final : public LResource
{
public:
    Wayland::RWlSurface *surfaceRes() const noexcept { return m_surfaceRes; }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_blur_region(wl_client *client, wl_resource *resource, wl_resource *region);

private:
    friend class GExtBackgroundEffectManager;
    RExtBackgroundEffectSurface(Wayland::RWlSurface *surfaceRes, UInt32 id, Int32 version) noexcept;
    ~RExtBackgroundEffectSurface() noexcept;
    CZWeak<Wayland::RWlSurface> m_surfaceRes;
};

#endif // REXTBACKGROUNDEFFECTSURFACE_H
