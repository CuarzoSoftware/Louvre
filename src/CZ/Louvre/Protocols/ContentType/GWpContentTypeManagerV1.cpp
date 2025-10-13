#include <CZ/Louvre/Protocols/ContentType/content-type-v1.h>
#include <CZ/Louvre/Protocols/ContentType/GWpContentTypeManagerV1.h>
#include <CZ/Louvre/Protocols/ContentType/RWpContentTypeV1.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::ContentType;

static const struct wp_content_type_manager_v1_interface imp
{
    .destroy = &GWpContentTypeManagerV1::destroy,
    .get_surface_content_type = &GWpContentTypeManagerV1::get_surface_content_type
};

LGLOBAL_INTERFACE_IMP(GWpContentTypeManagerV1, LOUVRE_CONTENT_TYPE_MANAGER_VERSION, wp_content_type_manager_v1_interface)

bool GWpContentTypeManagerV1::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.ContentTypeManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.ContentTypeManager;
    return true;
}

GWpContentTypeManagerV1::GWpContentTypeManagerV1
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
    this->client()->imp()->contentTypeManagerGlobals.emplace_back(this);
}

GWpContentTypeManagerV1::~GWpContentTypeManagerV1() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->contentTypeManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GWpContentTypeManagerV1::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void GWpContentTypeManagerV1::get_surface_content_type(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->contentTypeRes())
    {
        surfaceRes->postError(
            WP_CONTENT_TYPE_MANAGER_V1_ERROR_ALREADY_CONSTRUCTED,
            "wl_surface already has a content type object.");
        return;
    }

    new RWpContentTypeV1(surfaceRes, id, wl_resource_get_version(resource));
}
