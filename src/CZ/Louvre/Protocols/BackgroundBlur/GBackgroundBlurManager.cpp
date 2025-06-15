#include <CZ/Louvre/Protocols/BackgroundBlur/lvr-background-blur.h>
#include <CZ/Louvre/Protocols/BackgroundBlur/GBackgroundBlurManager.h>
#include <CZ/Louvre/Protocols/BackgroundBlur/RBackgroundBlur.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::BackgroundBlur;

static const struct lvr_background_blur_manager_interface imp
{
    .get_background_blur = &GBackgroundBlurManager::get_background_blur,
    .destroy = &GBackgroundBlurManager::destroy,
};

void GBackgroundBlurManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GBackgroundBlurManager(client, version, id);
}

Int32 GBackgroundBlurManager::maxVersion() noexcept
{
    return LOUVRE_BACKGROUND_BLUR_MANAGER_VERSION;
}

const wl_interface *GBackgroundBlurManager::interface() noexcept
{
    return &lvr_background_blur_manager_interface;
}

GBackgroundBlurManager::GBackgroundBlurManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
        ) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->backgroundBlurManagerGlobals.emplace_back(this);
    m_maskingCapabilities = LBackgroundBlur::maskingCapabilities;
    lvr_background_blur_manager_send_masking_capabilities(resource(), LBackgroundBlur::maskingCapabilities.get());
}

GBackgroundBlurManager::~GBackgroundBlurManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->backgroundBlurManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GBackgroundBlurManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GBackgroundBlurManager::get_background_blur(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    auto *res { static_cast<GBackgroundBlurManager*>(wl_resource_get_user_data(resource)) };
    auto *surfaceRes { static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->contentTypeRes())
    {
        res->postError(
            LVR_BACKGROUND_BLUR_MANAGER_ERROR_ALREADY_CONSTRUCTED,
            "the surface already has an associated background blur object");
        return;
    }

    new RBackgroundBlur(res->m_maskingCapabilities, surfaceRes, id, wl_resource_get_version(resource));
}
