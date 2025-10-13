#include <CZ/Louvre/Protocols/ImageCaptureSource/GForeignToplevelImageCaptureSourceManager.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/ext-image-capture-source-v1.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/RImageCaptureSource.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ::Protocols::ImageCaptureSource;
using namespace CZ;

static const struct ext_foreign_toplevel_image_capture_source_manager_v1_interface imp
{
    .create_source = &GForeignToplevelImageCaptureSourceManager::create_source,
    .destroy = &GForeignToplevelImageCaptureSourceManager::destroy
};

LGLOBAL_INTERFACE_IMP(GForeignToplevelImageCaptureSourceManager, LOUVRE_FOREIGN_TOPLEVEL_IMAGE_CAPTURE_SOURCE_MANAGER, ext_foreign_toplevel_image_capture_source_manager_v1_interface)

bool GForeignToplevelImageCaptureSourceManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.ForeignToplevelImageCaptureSourceManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.ForeignToplevelImageCaptureSourceManager;
    return true;
}

GForeignToplevelImageCaptureSourceManager::GForeignToplevelImageCaptureSourceManager(
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
    this->client()->imp()->foreignToplevelImageCaptureSourceManagerGlobals.emplace_back(this);
}

GForeignToplevelImageCaptureSourceManager::~GForeignToplevelImageCaptureSourceManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->foreignToplevelImageCaptureSourceManagerGlobals, this);
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
