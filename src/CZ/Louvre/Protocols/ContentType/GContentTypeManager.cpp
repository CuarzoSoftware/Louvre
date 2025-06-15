#include <CZ/Louvre/Protocols/ContentType/content-type-v1.h>
#include <CZ/Louvre/Protocols/ContentType/GContentTypeManager.h>
#include <CZ/Louvre/Protocols/ContentType/RContentType.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::ContentType;

static const struct wp_content_type_manager_v1_interface imp
{
    .destroy = &GContentTypeManager::destroy,
    .get_surface_content_type = &GContentTypeManager::get_surface_content_type
};

void GContentTypeManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GContentTypeManager(client, version, id);
}

Int32 GContentTypeManager::maxVersion() noexcept
{
    return LOUVRE_CONTENT_TYPE_MANAGER_VERSION;
}

const wl_interface *GContentTypeManager::interface() noexcept
{
    return &wp_content_type_manager_v1_interface;
}

GContentTypeManager::GContentTypeManager
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
    this->client()->imp()->contentTypeManagerGlobals.emplace_back(this);
}

GContentTypeManager::~GContentTypeManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->contentTypeManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GContentTypeManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GContentTypeManager::get_surface_content_type(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    auto *surfaceRes { static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->contentTypeRes())
    {
        surfaceRes->postError(
            WP_CONTENT_TYPE_MANAGER_V1_ERROR_ALREADY_CONSTRUCTED,
            "wl_surface already has a content type object.");
        return;
    }

    new RContentType(surfaceRes, id, wl_resource_get_version(resource));
}
