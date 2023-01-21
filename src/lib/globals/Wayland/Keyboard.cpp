#include <private/LClientPrivate.h>

#include <globals/Wayland/Keyboard.h>

#include <stdio.h>

void Louvre::Globals::Keyboard::resource_destroy(wl_resource *resource)
{
    LClient *client = (LClient*)wl_resource_get_user_data(resource);
    client->imp()->keyboardResource = nullptr;
}

#if LOUVRE_SEAT_VERSION >= 3
void Louvre::Globals::Keyboard::release(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}
#endif
