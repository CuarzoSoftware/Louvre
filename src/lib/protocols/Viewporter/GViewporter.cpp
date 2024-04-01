#include <protocols/Viewporter/viewporter.h>
#include <protocols/Viewporter/GViewporter.h>
#include <protocols/Viewporter/RViewport.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::Viewporter;

static const struct wp_viewporter_interface imp
{
    .destroy = &GViewporter::destroy,
    .get_viewport = &GViewporter::get_viewport
};

void GViewporter::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GViewporter(client, version, id);
}

Int32 GViewporter::maxVersion() noexcept
{
    return LOUVRE_VIEWPORTER_VERSION;
}

const wl_interface *GViewporter::interface() noexcept
{
    return &wp_viewporter_interface;
}

GViewporter::GViewporter
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
    this->client()->imp()->viewporterGlobals.push_back(this);
}

GViewporter::~GViewporter() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->viewporterGlobals, this);
}

/******************** REQUESTS ********************/

void GViewporter::get_viewport(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    auto *surfaceRes { static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->viewportRes())
    {
        wl_resource_post_error(resource, WP_VIEWPORTER_ERROR_VIEWPORT_EXISTS, "The surface already has a viewport object associated.");
        return;
    }

    new RViewport(surfaceRes, wl_resource_get_version(resource), id);
}

void GViewporter::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
