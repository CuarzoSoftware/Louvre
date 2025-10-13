#include <CZ/Louvre/Protocols/ImageCaptureSource/GOutputImageCaptureSourceManager.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/ext-image-capture-source-v1.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/RImageCaptureSource.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::ImageCaptureSource;
using namespace CZ;

static const struct ext_output_image_capture_source_manager_v1_interface imp
{
    .create_source = &GOutputImageCaptureSourceManager::create_source,
    .destroy = &GOutputImageCaptureSourceManager::destroy
};

LGLOBAL_INTERFACE_IMP(GOutputImageCaptureSourceManager, LOUVRE_OUTPUT_IMAGE_CAPTURE_SOURCE_MANAGER, ext_output_image_capture_source_manager_v1_interface)

bool GOutputImageCaptureSourceManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.OutputImageCaptureSourceManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.OutputImageCaptureSourceManager;
    return true;
}

GOutputImageCaptureSourceManager::GOutputImageCaptureSourceManager(
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
    this->client()->imp()->outputImageCaptureSourceManagerGlobals.emplace_back(this);
}

GOutputImageCaptureSourceManager::~GOutputImageCaptureSourceManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->outputImageCaptureSourceManagerGlobals, this);
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
