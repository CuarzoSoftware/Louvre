#include <protocols/Wayland/GSubcompositor.h>
#include <protocols/Wayland/RSubsurface.h>
#include <private/LSurfacePrivate.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::Wayland;

static const struct wl_subcompositor_interface imp
{
    .destroy = &GSubcompositor::destroy,
    .get_subsurface = &GSubcompositor::get_subsurface
};

GSubcompositor::GSubcompositor
    (
        wl_client *client,
        Int32 version,
        UInt32 id) noexcept
    :LResource
    (
        client,
        &wl_subcompositor_interface,
        version,
        id,
        &imp
        )
{
    this->client()->imp()->subcompositorGlobals.push_back(this);
}

GSubcompositor::~GSubcompositor() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->subcompositorGlobals, this);
}

/******************** REQUESTS ********************/

void GSubcompositor::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GSubcompositor(client, version, id);
}

void GSubcompositor::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GSubcompositor::get_subsurface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *parent) noexcept
{
    if (surface == parent)
    {
        wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_PARENT, "Invalid wl_subsurface parent.");
        return;
    }

    auto &surfaceRes { *static_cast<RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes.surface()->imp()->hasRoleOrPendingRole())
    {
        wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "Given wl_surface already has another role.");
        return;
    }

    auto &parentRes { *static_cast<RSurface*>(wl_resource_get_user_data(parent)) };

    if (surfaceRes.surface()->imp()->isInChildrenOrPendingChildren(parentRes.surface()))
    {
        wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_PARENT, "Parent can not be child of surface.");
        return;
    }

    new RSubsurface(static_cast<GSubcompositor*>(wl_resource_get_user_data(resource)),
                    surfaceRes.surface(),
                    parentRes.surface(),
                    id);
}
