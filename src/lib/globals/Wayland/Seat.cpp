#include <globals/Wayland/Seat.h>
#include <globals/Wayland/Pointer.h>
#include <globals/Wayland/Keyboard.h>

#include <private/LClientPrivate.h>

#include <LCompositor.h>
#include <LKeyboard.h>
#include <LSeat.h>

#include <sys/mman.h>

using namespace Louvre::Globals;

static struct wl_pointer_interface pointer_implementation =
{
    .set_cursor = &Pointer::set_cursor,
#if LOUVRE_SEAT_VERSION >= 3
    .release = &Pointer::release
#endif
};

static struct wl_keyboard_interface keyboard_implementation =
{
#if LOUVRE_SEAT_VERSION >= 3
    .release = &Keyboard::release
#endif
};

static struct wl_seat_interface seat_implementation =
{
    .get_pointer = &Seat::get_pointer,
    .get_keyboard = &Seat::get_keyboard,
    .get_touch = &Seat::get_touch,

#if LOUVRE_SEAT_VERSION >= 5
    .release = &Seat::release
#endif

};

void Seat::resource_destroy(wl_resource *resource)
{
    //printf("SEAT DESTROYED.\n");
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);
    lClient->imp()->seatResource = nullptr;
}

void Seat::get_pointer(wl_client *client, wl_resource *resource, UInt32 id)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    if(!(lClient->seat()->capabilities() & LSeat::Pointer))
    {
        wl_resource_post_error(resource,WL_SEAT_ERROR_MISSING_CAPABILITY,"get_pointer called on seat without the matching capability.");
        return;
    }

    Int32 version = wl_resource_get_version(resource);
    wl_resource *pointer = wl_resource_create(client, &wl_pointer_interface, version, id);
    wl_resource_set_implementation(pointer, &pointer_implementation, lClient, &Pointer::resource_destroy);
    lClient->imp()->pointerResource = pointer;
}

void Seat::get_keyboard (wl_client *client, wl_resource *resource, UInt32 id)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    if(!(lClient->seat()->capabilities() & LSeat::Keyboard))
    {
        wl_resource_post_error(resource, WL_SEAT_ERROR_MISSING_CAPABILITY, "get_keyboard called on seat without the matching capability.");
        return;
    }

    Int32 version = wl_resource_get_version(resource);
    lClient->imp()->keyboardResource  = wl_resource_create(client, &wl_keyboard_interface, version, id);
    wl_resource_set_implementation(lClient->keyboardResource() , &keyboard_implementation, lClient, &Keyboard::resource_destroy);

#if LOUVRE_SEAT_VERSION >= 4
    if(version >= 4)
        wl_keyboard_send_repeat_info(lClient->keyboardResource(), lClient->seat()->keyboard()->repeatRate(), lClient->seat()->keyboard()->repeatDelay());
#endif

    // TODO: CHECK v7 PRIVATE_MAP

    wl_keyboard_send_keymap(lClient->keyboardResource(), WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, lClient->seat()->keyboard()->keymapFd(), lClient->seat()->keyboard()->keymapSize());

}

void Seat::get_touch(wl_client *client, wl_resource *resource, UInt32 id)
{
    (void)client;(void)resource;(void)id;

    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    if(!(lClient->seat()->capabilities() & LSeat::Touch))
    {
        wl_resource_post_error(resource, WL_SEAT_ERROR_MISSING_CAPABILITY, "get_touch called on seat without the matching capability.");
        return;
    }
}

void Seat::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    LCompositor *lCompositor = (LCompositor*)data;

    LClient *lClient = nullptr;

    // Search for the client object
    for(LClient *c : lCompositor->clients())
    {
        if(c->client() == client)
        {
            lClient = c;
            break;
        }
    }

    if(!lClient)
        return;

    lClient->imp()->seatResource = wl_resource_create(client, &wl_seat_interface, version, id);
    wl_resource_set_implementation(lClient->seatResource(), &seat_implementation, lClient, &Seat::resource_destroy);
    wl_seat_send_capabilities(lClient->seatResource(), lClient->seat()->capabilities());

#if LOUVRE_SEAT_VERSION >= 2
    if(version >= 2)
        wl_seat_send_name(lClient->seatResource(), "seat0");
#endif

}

#if LOUVRE_SEAT_VERSION >= 5

    void Seat::release(wl_client *, wl_resource *resource)
    {
        wl_resource_destroy(resource);
    }

#endif
