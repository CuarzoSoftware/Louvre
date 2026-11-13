#include <CZ/Louvre/Protocols/BackgroundEffect/ext-background-effect-v1.h>
#include <CZ/Louvre/Protocols/BackgroundEffect/GBackgroundEffectManager.h>
#include <CZ/Louvre/Protocols/BackgroundEffect/RBackgroundEffect.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Roles/LBackgroundBlur.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::BackgroundEffect;

static const struct ext_background_effect_manager_v1_interface imp
{
    .destroy = &GBackgroundEffectManager::destroy,
    .get_background_effect = &GBackgroundEffectManager::get_background_effect,
};

LGLOBAL_INTERFACE_IMP(GBackgroundEffectManager, LOUVRE_BACKGROUND_EFFECT_MANAGER_VERSION, ext_background_effect_manager_v1_interface)

bool GBackgroundEffectManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.BackgroundEffectManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    // Same gating as the lvr background blur global: no compositor-side
    // renderer, no effect.
    if (RCore::Get()->asRS())
    {
        LLog(CZWarning, CZLN, "{} global disabled (using Raster GAPI)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.BackgroundEffectManager;
    return true;
}

GBackgroundEffectManager::GBackgroundEffectManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
        )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->backgroundEffectManagerGlobals.emplace_back(this);
    ext_background_effect_manager_v1_send_capabilities(resource(), EXT_BACKGROUND_EFFECT_MANAGER_V1_CAPABILITY_BLUR);
}

GBackgroundEffectManager::~GBackgroundEffectManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->backgroundEffectManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GBackgroundEffectManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GBackgroundEffectManager::get_background_effect(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    auto *res { static_cast<GBackgroundEffectManager*>(wl_resource_get_user_data(resource)) };
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };
    LBackgroundBlur &blur { *surfaceRes->surface()->backgroundBlur() };

    if (blur.backgroundEffectResource())
    {
        res->postError(
            EXT_BACKGROUND_EFFECT_MANAGER_V1_ERROR_BACKGROUND_EFFECT_EXISTS,
            "the surface already has a background effect object");
        return;
    }

    // Already using lvr_background_blur (e.g. through another toolkit): this one is inert
    new RBackgroundEffect(surfaceRes, id, wl_resource_get_version(resource), blur.backgroundBlurResource() != nullptr);
}
