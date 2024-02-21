#include <protocols/Wayland/private/GSeatPrivate.h>
#include <protocols/Wayland/RKeyboard.h>
#include <protocols/Wayland/RPointer.h>
#include <protocols/Wayland/RTouch.h>
#include <private/LClientPrivate.h>
#include <LSeat.h>

static struct wl_seat_interface seat_implementation =
{
    .get_pointer = &GSeat::GSeatPrivate::get_pointer,
    .get_keyboard = &GSeat::GSeatPrivate::get_keyboard,
    .get_touch = &GSeat::GSeatPrivate::get_touch,
#if LOUVRE_WL_SEAT_VERSION >= 5
    .release = &GSeat::GSeatPrivate::release
#endif
};

void GSeat::GSeatPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);

    new GSeat(client,
              &wl_seat_interface,
              version,
              id,
              &seat_implementation,
              &GSeat::GSeatPrivate::resource_destroy);
}

void GSeat::GSeatPrivate::resource_destroy(wl_resource *resource)
{
    delete (GSeat*)wl_resource_get_user_data(resource);
}

void GSeat::GSeatPrivate::get_pointer(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);

    GSeat *gSeat { (GSeat*)wl_resource_get_user_data(resource) };

    if (!(seat()->inputCapabilities() & LSeat::Pointer))
    {
        wl_resource_post_error(resource,WL_SEAT_ERROR_MISSING_CAPABILITY, "get_pointer called on seat without the matching capability.");
        return;
    }

    new RPointer(gSeat, id);
}

void GSeat::GSeatPrivate::get_keyboard(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);

    GSeat *gSeat { (GSeat*)wl_resource_get_user_data(resource) };

    if (!(seat()->inputCapabilities() & LSeat::Keyboard))
    {
        wl_resource_post_error(resource, WL_SEAT_ERROR_MISSING_CAPABILITY, "get_keyboard called on seat without the matching capability.");
        return;
    }

    new RKeyboard(gSeat, id);
}

void GSeat::GSeatPrivate::get_touch(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);
    L_UNUSED(id);

    GSeat *gSeat { (GSeat*)wl_resource_get_user_data(resource) };

    if (!(seat()->inputCapabilities() & LSeat::Touch))
    {
        wl_resource_post_error(resource, WL_SEAT_ERROR_MISSING_CAPABILITY, "get_touch called on seat without the matching capability.");
        return;
    }

    new RTouch(gSeat, id);
}

#if LOUVRE_WL_SEAT_VERSION >= 5
void GSeat::GSeatPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
