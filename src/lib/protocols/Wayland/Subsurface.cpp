#include "Subsurface.h"
#include <protocols/Wayland/private/SurfaceResourcePrivate.h>
#include <private/LSubsurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LCompositor.h>
#include <LLog.h>

void Louvre::Globals::Subsurface::resource_destroy(wl_resource *resource)
{
    LSubsurfaceRole *lSubsurface = (LSubsurfaceRole*)wl_resource_get_user_data(resource);
    delete lSubsurface;
}

void Louvre::Globals::Subsurface::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void Louvre::Globals::Subsurface::set_position(wl_client *client, wl_resource *resource, Int32 x, Int32 y)
{
    L_UNUSED(client);

    LSubsurfaceRole *lSubsurface = (LSubsurfaceRole*)wl_resource_get_user_data(resource);
    lSubsurface->imp()->pendingLocalPosS = LPoint(x,y);
    lSubsurface->imp()->hasPendingLocalPos = true;
}

void Louvre::Globals::Subsurface::place_above(wl_client *client, wl_resource *resource, wl_resource *sibiling)
{
    L_UNUSED(client);

    LSubsurfaceRole *lSubsurface = (LSubsurfaceRole*)wl_resource_get_user_data(resource);

    Protocols::Wayland::SurfaceResource *lSibilingResource = (Protocols::Wayland::SurfaceResource*)wl_resource_get_user_data(sibiling);
    LSurface *lSibiling = lSibilingResource->surface();

    for(LSurface *sib : lSubsurface->surface()->parent()->children())
    {
        if(sib == lSibiling)
        {
            lSubsurface->imp()->pendingPlaceAbove = sib;
            return;
        }
    }

    wl_resource_post_error(resource, WL_SUBSURFACE_ERROR_BAD_SURFACE, "wl_surface is not a sibling or the parent.");
}

void Louvre::Globals::Subsurface::place_below(wl_client *client, wl_resource *resource, wl_resource *sibiling)
{
    L_UNUSED(client);

    LSubsurfaceRole *lSubsurface = (LSubsurfaceRole*)wl_resource_get_user_data(resource);
    Protocols::Wayland::SurfaceResource *lSibilingResource = (Protocols::Wayland::SurfaceResource*)wl_resource_get_user_data(sibiling);
    LSurface *lSibiling = lSibilingResource->surface();

    for(LSurface *sib : lSubsurface->surface()->parent()->children())
    {
        if(sib == lSibiling)
        {
            lSubsurface->imp()->pendingPlaceBelow = sib;
            return;
        }
    }

    wl_resource_post_error(resource, WL_SUBSURFACE_ERROR_BAD_SURFACE, "wl_surface is not a sibling or the parent.");
}

void Louvre::Globals::Subsurface::set_sync(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    LSubsurfaceRole *lSubsurface = (LSubsurfaceRole*)wl_resource_get_user_data(resource);
    lSubsurface->imp()->isSynced = true;
    lSubsurface->syncModeChanged();
}

void Louvre::Globals::Subsurface::set_desync(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    LSubsurfaceRole *lSubsurface = (LSubsurfaceRole*)wl_resource_get_user_data(resource);
    lSubsurface->imp()->isSynced = false;
    lSubsurface->syncModeChanged();
    Protocols::Wayland::SurfaceResource::SurfaceResourcePrivate::apply_commit(lSubsurface->surface());
}
