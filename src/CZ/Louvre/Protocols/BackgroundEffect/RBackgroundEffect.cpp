#include <CZ/Louvre/Protocols/BackgroundEffect/ext-background-effect-v1.h>
#include <CZ/Louvre/Protocols/BackgroundEffect/RBackgroundEffect.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RRegion.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Roles/LBackgroundBlur.h>

using namespace CZ::Protocols::BackgroundEffect;

static const struct ext_background_effect_surface_v1_interface imp
{
    .destroy = &RBackgroundEffect::destroy,
    .set_blur_region = &RBackgroundEffect::set_blur_region,
};

RBackgroundEffect::RBackgroundEffect(
    Wayland::RWlSurface *surfaceRes,
    UInt32 id,
    Int32 version,
    bool inert
    ) noexcept
    :LResource
    (
        surfaceRes->client(),
        &ext_background_effect_surface_v1_interface,
        version,
        id,
        &imp
    ),
    m_surfaceRes(surfaceRes),
    m_inert(inert)
{
    if (m_inert)
        return;

    auto &blur { *surfaceRes->surface()->backgroundBlur() };
    blur.m_backgroundEffectRes.reset(this);
    blur.m_sentConfigurations.clear();

    // A previous object was destroyed without a commit since: start clean now (and not on the
    // next commit, which would drop what this object sets until then)
    if (blur.m_flags.has(LBackgroundBlur::Destroyed))
    {
        blur.m_flags.remove(LBackgroundBlur::Destroyed);
        blur.reset();
    }

    // No handshake: whatever state the compositor sets is applied right away
    blur.configureRequest();
}

RBackgroundEffect::~RBackgroundEffect() noexcept
{
    // The effect is removed on the next commit
    if (!m_inert && surfaceRes())
        surfaceRes()->surface()->backgroundBlur()->m_flags.add(LBackgroundBlur::Destroyed);
}

/******************** REQUESTS ********************/

void RBackgroundEffect::destroy(wl_client */*client*/, wl_resource *resource)
{
    // Always allowed (the object is just inert once its surface is gone)
    wl_resource_destroy(resource);
}

void RBackgroundEffect::set_blur_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &res { *static_cast<RBackgroundEffect*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        res.postError(EXT_BACKGROUND_EFFECT_SURFACE_V1_ERROR_SURFACE_DESTROYED, "the associated surface has been destroyed");
        return;
    }

    if (res.inert())
        return;

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };

    // A NULL region removes the effect: an empty region (the state is up to the compositor)
    if (region)
    {
        auto &rRegion { *static_cast<Protocols::Wayland::RRegion*>(wl_resource_get_user_data(region)) };
        blur.pendingProps().region = rRegion.region();
    }
    else
        blur.pendingProps().region.setEmpty();

    blur.pendingProps().isEmpty = blur.pendingProps().region.isEmpty();
    blur.pendingProps().isFullSize = false;
    blur.m_flags.add(LBackgroundBlur::RegionModified);
}
