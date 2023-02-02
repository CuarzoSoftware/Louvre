#include <protocols/Wayland/private/KeyboardResourcePrivate.h>

void KeyboardResource::KeyboardResourcePrivate::resource_destroy(wl_resource *resource)
{
    KeyboardResource *keyboardResource = (KeyboardResource*)wl_resource_get_user_data(resource);
    delete keyboardResource;
}

#if LOUVRE_SEAT_VERSION >= WL_KEYBOARD_RELEASE_SINCE_VERSION
void KeyboardResource::KeyboardResourcePrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
