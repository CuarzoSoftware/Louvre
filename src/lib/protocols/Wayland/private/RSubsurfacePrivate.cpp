#include <protocols/Wayland/private/RSubsurfacePrivate.h>
#include <protocols/Wayland/private/RSurfacePrivate.h>

#include <private/LSubsurfaceRolePrivate.h>

#include <LSurface.h>

void RSubsurface::RSubsurfacePrivate::resource_destroy(wl_resource *resource)
{
    RSubsurface *rSubsurface = (RSubsurface*)wl_resource_get_user_data(resource);
    delete rSubsurface;
}

void RSubsurface::RSubsurfacePrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RSubsurface::RSubsurfacePrivate::set_position(wl_client *client, wl_resource *resource, Int32 x, Int32 y)
{
    L_UNUSED(client);
    RSubsurface *rSubsurface = (RSubsurface*)wl_resource_get_user_data(resource);
    rSubsurface->subsurfaceRole()->imp()->pendingLocalPosS = LPoint(x,y);
    rSubsurface->subsurfaceRole()->imp()->hasPendingLocalPos = true;
}

void RSubsurface::RSubsurfacePrivate::place_above(wl_client *client, wl_resource *resource, wl_resource *sibiling)
{
    L_UNUSED(client);
    RSubsurface *rSubsurface = (RSubsurface*)wl_resource_get_user_data(resource);
    RSurface *rSibiling = (RSurface*)wl_resource_get_user_data(sibiling);

    for (LSurface *sib : rSubsurface->subsurfaceRole()->surface()->parent()->children())
    {
        if (sib == rSibiling->surface())
        {
            rSubsurface->subsurfaceRole()->imp()->pendingPlaceAbove = sib;
            return;
        }
    }

    wl_resource_post_error(resource, WL_SUBSURFACE_ERROR_BAD_SURFACE, "Subsurface is not sibling.");
}

void RSubsurface::RSubsurfacePrivate::place_below(wl_client *client, wl_resource *resource, wl_resource *sibiling)
{
    L_UNUSED(client);
    RSubsurface *rSubsurface = (RSubsurface*)wl_resource_get_user_data(resource);
    RSurface *rSibiling = (RSurface*)wl_resource_get_user_data(sibiling);

    for (LSurface *sib : rSubsurface->subsurfaceRole()->surface()->parent()->children())
    {
        if (sib == rSibiling->surface())
        {
            rSubsurface->subsurfaceRole()->imp()->pendingPlaceBelow = sib;
            return;
        }
    }

    wl_resource_post_error(resource, WL_SUBSURFACE_ERROR_BAD_SURFACE, "Subsurface is not sibling.");
}

void RSubsurface::RSubsurfacePrivate::set_sync(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RSubsurface *rSubsurface = (RSubsurface*)wl_resource_get_user_data(resource);
    rSubsurface->subsurfaceRole()->imp()->isSynced = true;
    rSubsurface->subsurfaceRole()->syncModeChanged();
}

void RSubsurface::RSubsurfacePrivate::set_desync(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RSubsurface *rSubsurface = (RSubsurface*)wl_resource_get_user_data(resource);
    rSubsurface->subsurfaceRole()->imp()->isSynced = false;
    rSubsurface->subsurfaceRole()->syncModeChanged();
    RSurface::RSurfacePrivate::apply_commit(rSubsurface->subsurfaceRole()->surface());
}
