#include <protocols/PresentationTime/private/GPresentationPrivate.h>
#include <protocols/PresentationTime/RPresentationFeedback.h>
#include <protocols/PresentationTime/presentation-time.h>
#include <protocols/Wayland/RSurface.h>

struct wp_presentation_interface presentation_implementation =
{
    .destroy = &GPresentation::GPresentationPrivate::destroy,
    .feedback = &GPresentation::GPresentationPrivate::feedback
};

void GPresentation::GPresentationPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);
    new GPresentation(client,
                        &wp_presentation_interface,
                        version,
                        id,
                        &presentation_implementation);
}

void GPresentation::GPresentationPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void GPresentation::GPresentationPrivate::feedback(wl_client *client, wl_resource *resource, wl_resource *surface, UInt32 id)
{
    L_UNUSED(client);
    GPresentation *gPresentation { static_cast<GPresentation*>(wl_resource_get_user_data(resource)) };
    Wayland::RSurface *rSurface { static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };
    new RPresentationFeedback(gPresentation, rSurface->surface(), id);
}
