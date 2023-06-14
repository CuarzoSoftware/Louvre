#include <protocols/Wayland/private/RSubsurfacePrivate.h>
#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <private/LSubsurfaceRolePrivate.h>
#include <LSurface.h>
#include <LCompositor.h>

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

static void onPendingPlaceAboveDestroy(wl_listener *listener, void *data)
{
    L_UNUSED(listener);
    wl_resource *resource = (wl_resource*)data;
    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);

    for (LSurface *s : LCompositor::compositor()->surfaces())
    {
        if (s->roleId() == LSurface::Subsurface)
        {
            if (s->subsurface()->imp()->pendingPlaceAbove == rSurface->surface())
                s->subsurface()->imp()->pendingPlaceAbove = nullptr;
        }
    }
}

void RSubsurface::RSubsurfacePrivate::place_above(wl_client *client, wl_resource *resource, wl_resource *sibiling)
{
    L_UNUSED(client);
    RSubsurface *rSubsurface = (RSubsurface*)wl_resource_get_user_data(resource);
    RSurface *rSibiling = (RSurface*)wl_resource_get_user_data(sibiling);

    if (rSibiling->surface()->parent() == rSubsurface->subsurfaceRole()->surface()->parent() &&
        rSibiling->surface() != rSubsurface->subsurfaceRole()->surface())
    {
        if (rSubsurface->subsurfaceRole()->imp()->pendingPlaceAbove)
            wl_list_remove(&rSubsurface->subsurfaceRole()->imp()->pendingPlaceAboveDestroyListener.link);

        rSubsurface->subsurfaceRole()->imp()->pendingPlaceAbove = rSibiling->surface();
        rSubsurface->subsurfaceRole()->imp()->pendingPlaceAboveDestroyListener.notify = &onPendingPlaceAboveDestroy;
        wl_resource_add_destroy_listener(rSibiling->resource(),
                                         &rSubsurface->subsurfaceRole()->imp()->pendingPlaceAboveDestroyListener);
        return;
    }

    wl_resource_post_error(resource, WL_SUBSURFACE_ERROR_BAD_SURFACE, "Subsurface is not sibling.");
}

static void onPendingPlaceBelowDestroy(wl_listener *listener, void *data)
{
    L_UNUSED(listener);
    wl_resource *resource = (wl_resource*)data;
    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);

    for (LSurface *s : LCompositor::compositor()->surfaces())
    {
        if (s->roleId() == LSurface::Subsurface)
        {
            if (s->subsurface()->imp()->pendingPlaceBelow == rSurface->surface())
                s->subsurface()->imp()->pendingPlaceBelow = nullptr;
        }
    }
}
void RSubsurface::RSubsurfacePrivate::place_below(wl_client *client, wl_resource *resource, wl_resource *sibiling)
{
    L_UNUSED(client);
    RSubsurface *rSubsurface = (RSubsurface*)wl_resource_get_user_data(resource);
    RSurface *rSibiling = (RSurface*)wl_resource_get_user_data(sibiling);

    if (rSibiling->surface()->parent() == rSubsurface->subsurfaceRole()->surface()->parent() &&
        rSibiling->surface() != rSubsurface->subsurfaceRole()->surface())
    {
        if (rSubsurface->subsurfaceRole()->imp()->pendingPlaceBelow)
            wl_list_remove(&rSubsurface->subsurfaceRole()->imp()->pendingPlaceBelowDestroyListener.link);

        rSubsurface->subsurfaceRole()->imp()
            ->pendingPlaceBelow = rSibiling->surface();

        rSubsurface->subsurfaceRole()->imp()
            ->pendingPlaceBelowDestroyListener.notify = &onPendingPlaceBelowDestroy;

        wl_resource_add_destroy_listener(rSibiling->resource(),
                                         &rSubsurface->subsurfaceRole()->imp()->pendingPlaceBelowDestroyListener);
        return;
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
