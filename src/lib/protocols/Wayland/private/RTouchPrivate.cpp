#include <protocols/Wayland/private/RTouchPrivate.h>

void RTouch::RTouchPrivate::resource_destroy(wl_resource *resource)
{
    delete (RTouch*)wl_resource_get_user_data(resource);
}

#if LOUVRE_WL_SEAT_VERSION >= 3
void RTouch::RTouchPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
