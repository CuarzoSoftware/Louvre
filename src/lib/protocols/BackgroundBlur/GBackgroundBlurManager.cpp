#include <protocols/BackgroundBlur/background-blur.h>
#include <protocols/BackgroundBlur/GBackgroundBlurManager.h>
#include <protocols/BackgroundBlur/RBackgroundBlur.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::BackgroundBlur;

static const struct background_blur_manager_interface imp
{
    .destroy = &GBackgroundBlurManager::destroy,
    .get_background_blur = &GBackgroundBlurManager::get_background_blur
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
    return &background_blur_manager_interface;
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
    auto *surfaceRes { static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->contentTypeRes())
    {
        wl_resource_post_error(resource,
                               BACKGROUND_BLUR_MANAGER_ERROR_ALREADY_CONSTRUCTED,
                               "the surface already has an associated background blur object");
        return;
    }

    new RBackgroundBlur(surfaceRes, id, wl_resource_get_version(resource));
}
