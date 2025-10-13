#include <CZ/Louvre/Protocols/BackgroundBlur/lvr-background-blur.h>
#include <CZ/Louvre/Protocols/BackgroundBlur/GBackgroundBlurManager.h>
#include <CZ/Louvre/Protocols/BackgroundBlur/RBackgroundBlur.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::BackgroundBlur;

static const struct lvr_background_blur_manager_interface imp
{
    .get_background_blur = &GBackgroundBlurManager::get_background_blur,
    .destroy = &GBackgroundBlurManager::destroy,
};

LGLOBAL_INTERFACE_IMP(GBackgroundBlurManager, LOUVRE_BACKGROUND_BLUR_MANAGER_VERSION, lvr_background_blur_manager_interface)

bool GBackgroundBlurManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.LvrBackgroundBlurManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    if (RCore::Get()->asRS())
    {
        LLog(CZWarning, CZLN, "{} global disabled (using Raster GAPI)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.LvrBackgroundBlurManager;
    return true;
}

GBackgroundBlurManager::GBackgroundBlurManager
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
    this->client()->imp()->backgroundBlurManagerGlobals.emplace_back(this);
    m_maskingCapabilities = LBackgroundBlur::MaskCaps;
    lvr_background_blur_manager_send_masking_capabilities(resource(), LBackgroundBlur::MaskCaps.get());
}

GBackgroundBlurManager::~GBackgroundBlurManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->backgroundBlurManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GBackgroundBlurManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GBackgroundBlurManager::get_background_blur(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    auto *res { static_cast<GBackgroundBlurManager*>(wl_resource_get_user_data(resource)) };
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->contentTypeRes())
    {
        res->postError(
            LVR_BACKGROUND_BLUR_MANAGER_ERROR_ALREADY_CONSTRUCTED,
            "the surface already has an associated background blur object");
        return;
    }

    new RBackgroundBlur(res->m_maskingCapabilities, surfaceRes, id, wl_resource_get_version(resource));
}
