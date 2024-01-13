#include <protocols/Viewporter/viewporter.h>
#include <protocols/Viewporter/private/RViewportPrivate.h>

void RViewport::RViewportPrivate::resource_destroy(wl_resource *resource)
{
    delete (RViewport*)wl_resource_get_user_data(resource);
}

void RViewport::RViewportPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RViewport::RViewportPrivate::set_source(wl_client *client, wl_resource *resource, Float24 x, Float24 y, Float24 width, Float24 height)
{
    L_UNUSED(client)
    RViewport *rViewport = (RViewport*)wl_resource_get_user_data(resource);

    if (!rViewport->surfaceResource())
    {
        wl_resource_post_error(resource, WP_VIEWPORT_ERROR_NO_SURFACE, "The wl_surface was destroyed.");
        return;
    }

    rViewport->imp()->srcRect.setX(wl_fixed_to_double(x));
    rViewport->imp()->srcRect.setY(wl_fixed_to_double(y));
    rViewport->imp()->srcRect.setW(wl_fixed_to_double(width));
    rViewport->imp()->srcRect.setH(wl_fixed_to_double(height));
}

void RViewport::RViewportPrivate::set_destination(wl_client *client, wl_resource *resource, Int32 width, Int32 height)
{
    L_UNUSED(client)
    RViewport *rViewport = (RViewport*)wl_resource_get_user_data(resource);

    if (!rViewport->surfaceResource())
    {
        wl_resource_post_error(resource, WP_VIEWPORT_ERROR_NO_SURFACE, "The wl_surface was destroyed.");
        return;
    }

    rViewport->imp()->dstSize.setW(width);
    rViewport->imp()->dstSize.setH(height);
}
