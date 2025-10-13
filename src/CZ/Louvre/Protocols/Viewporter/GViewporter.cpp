#include <CZ/Louvre/Protocols/Viewporter/viewporter.h>
#include <CZ/Louvre/Protocols/Viewporter/GViewporter.h>
#include <CZ/Louvre/Protocols/Viewporter/RViewport.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::Viewporter;

static const struct wp_viewporter_interface imp
{
    .destroy = &GViewporter::destroy,
    .get_viewport = &GViewporter::get_viewport
};

LGLOBAL_INTERFACE_IMP(GViewporter, LOUVRE_VIEWPORTER_VERSION, wp_viewporter_interface)

bool GViewporter::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.Viewporter)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.Viewporter;
    return true;
}

GViewporter::GViewporter
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
    this->client()->imp()->viewporterGlobals.push_back(this);
}

GViewporter::~GViewporter() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->viewporterGlobals, this);
}

/******************** REQUESTS ********************/

void GViewporter::get_viewport(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->viewportRes())
    {
        surfaceRes->postError(WP_VIEWPORTER_ERROR_VIEWPORT_EXISTS, "The surface already has a viewport object associated.");
        return;
    }

    new RViewport(surfaceRes, wl_resource_get_version(resource), id);
}

void GViewporter::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
