#include <CZ/Louvre/Protocols/Wayland/GSubcompositor.h>
#include <CZ/Louvre/Protocols/Wayland/RSubsurface.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::Wayland;

static const struct wl_subcompositor_interface imp
{
    .destroy = &GSubcompositor::destroy,
    .get_subsurface = &GSubcompositor::get_subsurface
};

void GSubcompositor::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GSubcompositor(client, version, id);
}

Int32 GSubcompositor::maxVersion() noexcept
{
    return LOUVRE_WL_SUBCOMPOSITOR_VERSION;
}

const wl_interface *GSubcompositor::interface() noexcept
{
    return &wl_subcompositor_interface;
}

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

void GSubcompositor::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GSubcompositor::get_subsurface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *parent) noexcept
{
    auto &surfaceRes { *static_cast<RSurface*>(wl_resource_get_user_data(surface)) };
    auto &parentRes { *static_cast<RSurface*>(wl_resource_get_user_data(parent)) };

    if (surface == parent)
    {
        parentRes.postError(WL_SUBCOMPOSITOR_ERROR_BAD_PARENT, "Invalid wl_subsurface parent.");
        return;
    }

    if (surfaceRes.surface()->role())
    {
        parentRes.postError(WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "Given wl_surface already has another role.");
        return;
    }

    if (surfaceRes.surface()->imp()->isInChildrenOrPendingChildren(parentRes.surface()))
    {
        parentRes.postError(WL_SUBCOMPOSITOR_ERROR_BAD_PARENT, "Parent can not be child of surface.");
        return;
    }

    new RSubsurface(static_cast<GSubcompositor*>(wl_resource_get_user_data(resource)),
                    surfaceRes.surface(),
                    parentRes.surface(),
                    id);
}
