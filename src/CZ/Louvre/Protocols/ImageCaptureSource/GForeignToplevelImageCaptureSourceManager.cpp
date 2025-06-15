#include <CZ/Louvre/Protocols/ImageCaptureSource/GForeignToplevelImageCaptureSourceManager.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/ext-image-capture-source-v1.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/RImageCaptureSource.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::ImageCaptureSource;
using namespace Louvre;

static const struct ext_foreign_toplevel_image_capture_source_manager_v1_interface imp
{
    .create_source = &GForeignToplevelImageCaptureSourceManager::create_source,
    .destroy = &GForeignToplevelImageCaptureSourceManager::destroy
};

void GForeignToplevelImageCaptureSourceManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GForeignToplevelImageCaptureSourceManager(client, version, id);
}

Int32 GForeignToplevelImageCaptureSourceManager::maxVersion() noexcept
{
    return LOUVRE_FOREIGN_TOPLEVEL_IMAGE_CAPTURE_SOURCE_MANAGER;
}

const wl_interface *GForeignToplevelImageCaptureSourceManager::interface() noexcept
{
    return &ext_foreign_toplevel_image_capture_source_manager_v1_interface;
}

GForeignToplevelImageCaptureSourceManager::GForeignToplevelImageCaptureSourceManager(
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
    this->client()->imp()->foreignToplevelImageCaptureSourceManagerGlobals.emplace_back(this);
}

GForeignToplevelImageCaptureSourceManager::~GForeignToplevelImageCaptureSourceManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->foreignToplevelImageCaptureSourceManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GForeignToplevelImageCaptureSourceManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GForeignToplevelImageCaptureSourceManager::create_source(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *toplevel_handle) noexcept
{
    new RImageCaptureSource(
        static_cast<GForeignToplevelImageCaptureSourceManager*>(wl_resource_get_user_data(resource))->client(),
        wl_resource_get_version(resource),
        id,
        LImageCaptureSourceType::ForeignToplevel,
        static_cast<LResource*>(wl_resource_get_user_data(toplevel_handle)));
}
