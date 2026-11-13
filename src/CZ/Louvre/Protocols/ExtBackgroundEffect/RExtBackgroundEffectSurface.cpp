#include <CZ/Louvre/Protocols/ExtBackgroundEffect/ext-background-effect-v1.h>
#include <CZ/Louvre/Protocols/ExtBackgroundEffect/RExtBackgroundEffectSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RRegion.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Roles/LBackgroundBlur.h>

using namespace CZ::Protocols::ExtBackgroundEffect;

static const struct ext_background_effect_surface_v1_interface imp
{
    .destroy = &RExtBackgroundEffectSurface::destroy,
    .set_blur_region = &RExtBackgroundEffectSurface::set_blur_region,
};

RExtBackgroundEffectSurface::RExtBackgroundEffectSurface(
    Wayland::RWlSurface *surfaceRes,
    UInt32 id,
    Int32 version
    ) noexcept
    :LResource
    (
        surfaceRes->client(),
        &ext_background_effect_surface_v1_interface,
        version,
        id,
        &imp
    ),
    m_surfaceRes(surfaceRes)
{
    surfaceRes->surface()->backgroundBlur()->m_extBackgroundEffectRes.reset(this);
}

RExtBackgroundEffectSurface::~RExtBackgroundEffectSurface() noexcept
{
    if (surfaceRes())
    {
        auto &blur { *surfaceRes()->surface()->backgroundBlur() };
        // The effect regions are removed on the next commit.
        blur.m_flags.add(LBackgroundBlur::Destroyed);
    }
}

/******************** REQUESTS ********************/

void RExtBackgroundEffectSurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RExtBackgroundEffectSurface*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        res.postError(EXT_BACKGROUND_EFFECT_SURFACE_V1_ERROR_SURFACE_DESTROYED, "the associated surface has been destroyed");
        return;
    }

    wl_resource_destroy(resource);
}

void RExtBackgroundEffectSurface::set_blur_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &res { *static_cast<RExtBackgroundEffectSurface*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        res.postError(EXT_BACKGROUND_EFFECT_SURFACE_V1_ERROR_SURFACE_DESTROYED, "the associated surface has been destroyed");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };

    // No handshake in this protocol: a non-null region enables the
    // effect, a null one removes it. Applied on the next commit.
    if (region)
    {
        auto &rRegion { *static_cast<Protocols::Wayland::RRegion*>(wl_resource_get_user_data(region)) };
        blur.pendingProps().state = LBackgroundBlur::Enabled;
        blur.pendingProps().region = rRegion.region();
        blur.pendingProps().isEmpty = blur.pendingProps().region.isEmpty();
        blur.pendingProps().isFullSize = false;
        blur.m_flags.add(LBackgroundBlur::RegionModified);
    }
    else if (blur.pendingProps().state != LBackgroundBlur::Disabled)
    {
        blur.pendingProps().state = LBackgroundBlur::Disabled;
        blur.m_flags.add(LBackgroundBlur::RegionModified);
    }
}
