#include <protocols/WpPresentationTime/private/GWpPresentationPrivate.h>
#include <protocols/WpPresentationTime/RWpPresentationFeedback.h>
#include <protocols/WpPresentationTime/presentation-time.h>
#include <protocols/Wayland/RSurface.h>

struct wp_presentation_interface presentation_implementation =
{
    .destroy = &GWpPresentation::GWpPresentationPrivate::destroy,
    .feedback = &GWpPresentation::GWpPresentationPrivate::feedback
};

void GWpPresentation::GWpPresentationPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);
    new GWpPresentation(client,
                        &wp_presentation_interface,
                        version,
                        id,
                        &presentation_implementation,
                        &GWpPresentation::GWpPresentationPrivate::resource_destroy);
}

void GWpPresentation::GWpPresentationPrivate::resource_destroy(wl_resource *resource)
{
    GWpPresentation *gWpPresentation = (GWpPresentation*)wl_resource_get_user_data(resource);
    delete gWpPresentation;
}

void GWpPresentation::GWpPresentationPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void GWpPresentation::GWpPresentationPrivate::feedback(wl_client *client, wl_resource *resource, wl_resource *surface, UInt32 id)
{
    L_UNUSED(client);
    GWpPresentation *gWpPresentation = (GWpPresentation*)wl_resource_get_user_data(resource);
    Wayland::RSurface *rSurface = (Wayland::RSurface*)wl_resource_get_user_data(surface);
    new RWpPresentationFeedback(gWpPresentation, rSurface->surface(), id);
}
