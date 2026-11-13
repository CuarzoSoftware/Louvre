#ifndef GEXTBACKGROUNDEFFECTMANAGER_H
#define GEXTBACKGROUNDEFFECTMANAGER_H

#include <CZ/Louvre/LResource.h>

/**
 * @brief ext_background_effect_manager_v1 global.
 *
 * Front-door into the same per-surface state driven by
 * Protocols::BackgroundBlur (LBackgroundBlur), without that protocol's
 * configure/acknowledge handshake: the requested region is applied to
 * LSurface::backgroundBlur() on the next wl_surface.commit.
 */
class CZ::Protocols::ExtBackgroundEffect::GExtBackgroundEffectManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_background_effect(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);

private:
    LGLOBAL_INTERFACE
    GExtBackgroundEffectManager(wl_client *client, Int32 version, UInt32 id);
    ~GExtBackgroundEffectManager() noexcept;
};

#endif // GEXTBACKGROUNDEFFECTMANAGER_H
