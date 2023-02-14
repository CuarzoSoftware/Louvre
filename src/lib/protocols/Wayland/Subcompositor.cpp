#include <protocols/Wayland/Subcompositor.h>
#include <protocols/Wayland/Subsurface.h>

#include <private/LSubsurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>

#include <LCompositor.h>

#include <stdio.h>

using namespace Louvre;

struct wl_subsurface_interface subsurface_implementation =
{
    .destroy = &Globals::Subsurface::destroy,
    .set_position = &Globals::Subsurface::set_position,
    .place_above = &Globals::Subsurface::place_above,
    .place_below = &Globals::Subsurface::place_below,
    .set_sync = &Globals::Subsurface::set_sync,
    .set_desync = &Globals::Subsurface::set_desync
};

struct wl_subcompositor_interface subcompositor_implementation =
{
    .destroy = &Globals::Subcompositor::destroy,
    .get_subsurface = &Globals::Subcompositor::get_subsurface
};

void Globals::Subcompositor::resource_destroy(wl_resource *resource)
{
    L_UNUSED(resource);
}

void Globals::Subcompositor::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void Globals::Subcompositor::get_subsurface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *parent)
{
    Protocols::Wayland::SurfaceResource *lSurfaceResource = (Protocols::Wayland::SurfaceResource*)wl_resource_get_user_data(surface);
    LSurface *lSurface = lSurfaceResource->surface();

    if(lSurface->role())
    {
        wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "Given wl_surface already has another role.");
        return;
    }

    Int32 version = wl_resource_get_version(resource);
    LCompositor *lCompositor = (LCompositor*)wl_resource_get_user_data(resource);

    wl_resource *subsurface = wl_resource_create(client, &wl_subsurface_interface, version, id);

    LSubsurfaceRole::Params subsurfaceRoleParams;
    subsurfaceRoleParams.subsurface = subsurface;
    subsurfaceRoleParams.surface = lSurface;

    LSubsurfaceRole *lSubsurface = lCompositor->createSubsurfaceRoleRequest(&subsurfaceRoleParams);

    Protocols::Wayland::SurfaceResource *lParentResource = (Protocols::Wayland::SurfaceResource*)wl_resource_get_user_data(parent);
    LSurface *lParent = lParentResource->surface();
    lParent->imp()->pendingChildren.push_back(lSurface);
    lSurface->imp()->pendingParent = lParent;

    wl_resource_set_implementation(subsurface, &subsurface_implementation, lSubsurface, &Subsurface::resource_destroy);
    lSurface->imp()->setPendingRole(lSubsurface);
    lSurface->imp()->applyPendingRole();

    //printf("\n**** SUBSU %d PARET %d PAR ROLE %d\n", wl_resource_get_id(surface), wl_resource_get_id(parent), lParent->roleId());

}

void Globals::Subcompositor::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    LCompositor *lCompositor = (LCompositor*)data;
    wl_resource *subcompositor = wl_resource_create(client, &wl_subcompositor_interface, version, id);
    wl_resource_set_implementation(subcompositor, &subcompositor_implementation, lCompositor, &Subcompositor::resource_destroy);
}
