#include <protocols/Wayland/GCompositor.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/Wayland/RRegion.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LUtils.h>

using namespace Louvre::Protocols::Wayland;

static const struct wl_compositor_interface imp
{
    .create_surface = &GCompositor::create_surface,
    .create_region = &GCompositor::create_region
};

void GCompositor::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GCompositor(client, version, id);
}

Int32 GCompositor::maxVersion() noexcept
{
    return LOUVRE_WL_COMPOSITOR_VERSION;
}

const wl_interface *GCompositor::interface() noexcept
{
    return &wl_compositor_interface;
}

GCompositor::GCompositor
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
    this->client()->imp()->compositorGlobals.push_back(this);
}

GCompositor::~GCompositor() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->compositorGlobals, this);
}

/******************** REQUESTS ********************/

void GCompositor::create_surface(wl_client */*client*/, wl_resource *resource, UInt32 id)
{
    new RSurface(static_cast<GCompositor*>(wl_resource_get_user_data(resource)), id);
}

void GCompositor::create_region(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    new RRegion(static_cast<GCompositor*>(wl_resource_get_user_data(resource)), id);
}
