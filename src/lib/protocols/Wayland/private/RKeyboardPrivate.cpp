#include <protocols/Wayland/private/RKeyboardPrivate.h>

#if LOUVRE_WL_SEAT_VERSION >= 3
void RKeyboard::RKeyboardPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
