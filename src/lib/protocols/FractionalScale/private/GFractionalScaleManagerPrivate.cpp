#include <protocols/FractionalScale/private/GFractionalScaleManagerPrivate.h>
#include <protocols/FractionalScale/private/RFractionalScalePrivate.h>
#include <protocols/Wayland/RSurface.h>

static struct wp_fractional_scale_manager_v1_interface wp_fractional_scale_manager_v1_implementation =
{
    .destroy = &GFractionalScaleManager::GFractionalScaleManagerPrivate::destroy,
    .get_fractional_scale = &GFractionalScaleManager::GFractionalScaleManagerPrivate::get_fractional_scale
};

void GFractionalScaleManager::GFractionalScaleManagerPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);
    new GFractionalScaleManager(client,
                     &wp_fractional_scale_manager_v1_interface,
                     version,
                     id,
                     &wp_fractional_scale_manager_v1_implementation,
                     &GFractionalScaleManager::GFractionalScaleManagerPrivate::resource_destroy);
}

void GFractionalScaleManager::GFractionalScaleManagerPrivate::resource_destroy(wl_resource *resource)
{
    delete (GFractionalScaleManager*)wl_resource_get_user_data(resource);
}

void GFractionalScaleManager::GFractionalScaleManagerPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void GFractionalScaleManager::GFractionalScaleManagerPrivate::get_fractional_scale(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    L_UNUSED(client);
    Wayland::RSurface *rSurface = (Wayland::RSurface*)wl_resource_get_user_data(surface);

    if (rSurface->fractionalScaleResource())
    {
        wl_resource_post_error(resource,
                               WP_FRACTIONAL_SCALE_MANAGER_V1_ERROR_FRACTIONAL_SCALE_EXISTS,
                               "The surface already has a fractional_scale object associated.");
        return;
    }

    new RFractionalScale(rSurface, id, wl_resource_get_version(resource));
}

