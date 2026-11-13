#include <CZ/Louvre/Protocols/ExtBackgroundEffect/ext-background-effect-v1.h>
#include <CZ/Louvre/Protocols/ExtBackgroundEffect/GExtBackgroundEffectManager.h>
#include <CZ/Louvre/Protocols/ExtBackgroundEffect/RExtBackgroundEffectSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Roles/LBackgroundBlur.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::ExtBackgroundEffect;

static const struct ext_background_effect_manager_v1_interface imp
{
    .destroy = &GExtBackgroundEffectManager::destroy,
    .get_background_effect = &GExtBackgroundEffectManager::get_background_effect,
};

LGLOBAL_INTERFACE_IMP(GExtBackgroundEffectManager, LOUVRE_EXT_BACKGROUND_EFFECT_VERSION, ext_background_effect_manager_v1_interface)

bool GExtBackgroundEffectManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.ExtBackgroundEffectManager)
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

    *slot = &compositor()->wellKnownGlobals.ExtBackgroundEffectManager;
    return true;
}

GExtBackgroundEffectManager::GExtBackgroundEffectManager
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
    this->client()->imp()->extBackgroundEffectManagerGlobals.emplace_back(this);
    ext_background_effect_manager_v1_send_capabilities(resource(), EXT_BACKGROUND_EFFECT_MANAGER_V1_CAPABILITY_BLUR);
}

GExtBackgroundEffectManager::~GExtBackgroundEffectManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->extBackgroundEffectManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GExtBackgroundEffectManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GExtBackgroundEffectManager::get_background_effect(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    auto *res { static_cast<GExtBackgroundEffectManager*>(wl_resource_get_user_data(resource)) };
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };
    LBackgroundBlur &blur { *surfaceRes->surface()->backgroundBlur() };

    if (blur.backgroundEffectResource() != nullptr ||
        blur.backgroundBlurResource() != nullptr)
    {
        res->postError(
            EXT_BACKGROUND_EFFECT_MANAGER_V1_ERROR_BACKGROUND_EFFECT_EXISTS,
            "the surface already has a background effect object");
        return;
    }

    new RExtBackgroundEffectSurface(surfaceRes, id, wl_resource_get_version(resource));
}
