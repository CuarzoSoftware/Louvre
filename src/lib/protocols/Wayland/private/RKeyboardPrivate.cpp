#include <protocols/Wayland/private/RKeyboardPrivate.h>

void RKeyboard::RKeyboardPrivate::resource_destroy(wl_resource *resource)
{
    RKeyboard *keyboardResource = (RKeyboard*)wl_resource_get_user_data(resource);
    delete keyboardResource;
}

#if LOUVRE_WL_SEAT_VERSION >= WL_KEYBOARD_RELEASE_SINCE_VERSION
void RKeyboard::RKeyboardPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
