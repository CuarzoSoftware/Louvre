#include <CZ/Louvre/Protocols/ImageCaptureSource/GOutputImageCaptureSourceManager.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/ext-image-capture-source-v1.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/RImageCaptureSource.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::ImageCaptureSource;
using namespace Louvre;

static const struct ext_output_image_capture_source_manager_v1_interface imp
{
    .create_source = &GOutputImageCaptureSourceManager::create_source,
    .destroy = &GOutputImageCaptureSourceManager::destroy
};

void GOutputImageCaptureSourceManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GOutputImageCaptureSourceManager(client, version, id);
}

Int32 GOutputImageCaptureSourceManager::maxVersion() noexcept
{
    return LOUVRE_OUTPUT_IMAGE_CAPTURE_SOURCE_MANAGER;
}

const wl_interface *GOutputImageCaptureSourceManager::interface() noexcept
{
    return &ext_output_image_capture_source_manager_v1_interface;
}

GOutputImageCaptureSourceManager::GOutputImageCaptureSourceManager(
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
    this->client()->imp()->outputImageCaptureSourceManagerGlobals.emplace_back(this);
}

GOutputImageCaptureSourceManager::~GOutputImageCaptureSourceManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->outputImageCaptureSourceManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GOutputImageCaptureSourceManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GOutputImageCaptureSourceManager::create_source(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *output) noexcept
{
    new RImageCaptureSource(
        static_cast<GOutputImageCaptureSourceManager*>(wl_resource_get_user_data(resource))->client(),
        wl_resource_get_version(resource),
        id,
        LImageCaptureSourceType::Output,
        static_cast<LResource*>(wl_resource_get_user_data(output)));
}
