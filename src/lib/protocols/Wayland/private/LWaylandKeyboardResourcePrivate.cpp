#include <protocols/Wayland/private/LWaylandKeyboardResourcePrivate.h>

void LWaylandKeyboardResource::LWaylandKeyboardResourcePrivate::resource_destroy(wl_resource *resource)
{
    LWaylandKeyboardResource *keyboardResource = (LWaylandKeyboardResource*)wl_resource_get_user_data(resource);
    delete keyboardResource;
}

#if LOUVRE_SEAT_VERSION >= WL_KEYBOARD_RELEASE_SINCE_VERSION
void LWaylandKeyboardResource::LWaylandKeyboardResourcePrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
