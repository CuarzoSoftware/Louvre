#include <protocols/Viewporter/private/GViewporterPrivate.h>
#include <protocols/Viewporter/viewporter.h>
#include <protocols/Viewporter/RViewport.h>
#include <LCompositor.h>

struct wp_viewporter_interface viewporter_implementation =
{
    .destroy = &GViewporter::GViewporterPrivate::destroy,
    .get_viewport = &GViewporter::GViewporterPrivate::get_viewport
};

void GViewporter::GViewporterPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);

    LClient *lClient = compositor()->getClientFromNativeResource(client);
    new GViewporter(lClient,
                    &wp_viewporter_interface,
                    version,
                    id,
                    &viewporter_implementation,
                    &GViewporter::GViewporterPrivate::resource_destroy);
}

void GViewporter::GViewporterPrivate::get_viewport(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    L_UNUSED(client);

    Wayland::RSurface *rSurface = (Wayland::RSurface*)wl_resource_get_user_data(surface);

    if (rSurface->viewport())
    {
        wl_resource_post_error(resource, WP_VIEWPORTER_ERROR_VIEWPORT_EXISTS, "The surface already has a viewport object associated.");
        return;
    }

    new RViewport(rSurface, wl_resource_get_version(resource), id);
}


void GViewporter::GViewporterPrivate::resource_destroy(wl_resource *resource)
{
    delete (GViewporter*)wl_resource_get_user_data(resource);
}

void GViewporter::GViewporterPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client)
    wl_resource_destroy(resource);
}
