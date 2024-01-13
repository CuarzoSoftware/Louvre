#include <protocols/FractionalScale/private/RFractionalScalePrivate.h>

void RFractionalScale::RFractionalScalePrivate::resource_destroy(wl_resource *resource)
{
    delete (RFractionalScale*)wl_resource_get_user_data(resource);
}

void RFractionalScale::RFractionalScalePrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client)
    wl_resource_destroy(resource);
}
