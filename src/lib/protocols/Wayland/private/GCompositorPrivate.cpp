#include <protocols/Wayland/private/GCompositorPrivate.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/Wayland/RRegion.h>
#include <LCompositor.h>
#include <LLog.h>

struct wl_compositor_interface compositor_implementation =
{
    .create_surface = &GCompositor::GCompositorPrivate::create_surface,
    .create_region = &GCompositor::GCompositorPrivate::create_region
};

void GCompositor::GCompositorPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);

    LClient *lClient = compositor()->getClientFromNativeResource(client);

    if (lClient->compositorGlobal())
    {
        LLog::warning("Client bound twice to the wl_compositor singleton global. Ignoring it.");
        return;
    }

    new GCompositor(lClient,
                    &wl_compositor_interface,
                    version,
                    id,
                    &compositor_implementation,
                    &GCompositor::GCompositorPrivate::resource_destroy);
}

void GCompositor::GCompositorPrivate::create_surface(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);
    GCompositor *gCompositor = (GCompositor*)wl_resource_get_user_data(resource);
    new RSurface(gCompositor, id);
}

void GCompositor::GCompositorPrivate::create_region (wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);
    GCompositor *gCompositor = (GCompositor*)wl_resource_get_user_data(resource);
    new RRegion(gCompositor, id);
}

void GCompositor::GCompositorPrivate::resource_destroy(wl_resource *resource)
{
    GCompositor *gCompositor = (GCompositor*)wl_resource_get_user_data(resource);
    delete gCompositor;
}
