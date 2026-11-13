#ifndef GBACKGROUNDEFFECTMANAGER_H
#define GBACKGROUNDEFFECTMANAGER_H

#include <CZ/Louvre/LResource.h>

/**
 * @brief ext_background_effect_manager_v1 global.
 *
 * Front-door into the same per-surface state driven by
 * Protocols::BackgroundBlur (LBackgroundBlur), without that protocol's
 * configure/acknowledge handshake: the requested region is applied to
 * LSurface::backgroundBlur() on the next wl_surface.commit, the state set by
 * the compositor right away.
 */
class CZ::Protocols::BackgroundEffect::GBackgroundEffectManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_background_effect(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);

private:
    LGLOBAL_INTERFACE
    GBackgroundEffectManager(wl_client *client, Int32 version, UInt32 id);
    ~GBackgroundEffectManager() noexcept;
};

#endif // GBACKGROUNDEFFECTMANAGER_H
