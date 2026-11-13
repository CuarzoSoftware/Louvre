#ifndef RBACKGROUNDEFFECT_H
#define RBACKGROUNDEFFECT_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

/**
 * @brief ext_background_effect_surface_v1 resource.
 *
 * Sets the blur region of the surface's LBackgroundBlur (double-buffered, applied on commit). There
 * is no configure/acknowledge handshake: the state stays compositor-driven (see
 * LBackgroundBlur::configureState()), applied right away.
 *
 * Inert if the surface already uses the lvr_background_blur protocol.
 */
class CZ::Protocols::BackgroundEffect::RBackgroundEffect final : public LResource
{
public:
    Wayland::RWlSurface *surfaceRes() const noexcept { return m_surfaceRes; }

    /// Ignored: the surface already had a background blur object (lvr_background_blur).
    bool inert() const noexcept { return m_inert; }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_blur_region(wl_client *client, wl_resource *resource, wl_resource *region);

private:
    friend class GBackgroundEffectManager;
    RBackgroundEffect(Wayland::RWlSurface *surfaceRes, UInt32 id, Int32 version, bool inert) noexcept;
    ~RBackgroundEffect() noexcept;
    CZWeak<Wayland::RWlSurface> m_surfaceRes;
    bool m_inert;
};

#endif // RBACKGROUNDEFFECT_H
