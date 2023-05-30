#include <protocols/Wayland/private/GSubcompositorPrivate.h>
#include <protocols/Wayland/RSubsurface.h>
#include <protocols/Wayland/RSurface.h>

#include <LSurface.h>

struct wl_subcompositor_interface subcompositor_implementation =
{
    .destroy = &GSubcompositor::GSubcompositorPrivate::destroy,
    .get_subsurface = &GSubcompositor::GSubcompositorPrivate::get_subsurface
};

void GSubcompositor::GSubcompositorPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);

    new GSubcompositor(client,
                       &wl_subcompositor_interface,
                       version,
                       id,
                       &subcompositor_implementation,
                       &GSubcompositor::GSubcompositorPrivate::resource_destroy);
}

void GSubcompositor::GSubcompositorPrivate::resource_destroy(wl_resource *resource)
{
    GSubcompositor *lSubcompositor = (GSubcompositor*)wl_resource_get_user_data(resource);
    delete lSubcompositor;
}

void GSubcompositor::GSubcompositorPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void GSubcompositor::GSubcompositorPrivate::get_subsurface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *parent)
{
    L_UNUSED(client);
    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(surface);

    if (rSurface->surface()->role())
    {
        wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "Given wl_surface already has another role.");
        return;
    }

    GSubcompositor *gSubcompositor = (GSubcompositor*)wl_resource_get_user_data(resource);
    RSurface *rParent = (RSurface*)wl_resource_get_user_data(parent);
    new RSubsurface(gSubcompositor, rSurface->surface(), rParent->surface(), id);
}
