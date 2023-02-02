#include <protocols/Wayland/KeyboardResource.h>
#include <LSeat.h>
#include <private/LClientPrivate.h>
#include <protocols/Wayland/PointerResource.h>
#include <protocols/Wayland/private/SeatGlobalPrivate.h>

using namespace Louvre::Globals;


static struct wl_seat_interface seat_implementation =
{
    .get_pointer = &SeatGlobal::SeatGlobalPrivate::get_pointer,
    .get_keyboard = &SeatGlobal::SeatGlobalPrivate::get_keyboard,
    .get_touch = &SeatGlobal::SeatGlobalPrivate::get_touch,
#if LOUVRE_SEAT_VERSION >= WL_SEAT_RELEASE_SINCE_VERSION
    .release = &SeatGlobal::SeatGlobalPrivate::release
#endif
};

void SeatGlobal::SeatGlobalPrivate::bind(wl_client *client, void *compositor, UInt32 version, UInt32 id)
{
    new SeatGlobal((LCompositor*)compositor,
                            client,
                            &wl_seat_interface,
                            version,
                            id,
                            &seat_implementation,
                            &SeatGlobal::SeatGlobalPrivate::resource_destroy);
}

void SeatGlobal::SeatGlobalPrivate::resource_destroy(wl_resource *resource)
{
    SeatGlobal *seatResource = (SeatGlobal*)wl_resource_get_user_data(resource);
    delete seatResource;
}

void SeatGlobal::SeatGlobalPrivate::get_pointer(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);

    SeatGlobal *seatGlobal = (SeatGlobal*)wl_resource_get_user_data(resource);

    if(!(seatGlobal->client()->seat()->capabilities() & LSeat::Pointer))
    {
        wl_resource_post_error(resource,WL_SEAT_ERROR_MISSING_CAPABILITY,"get_pointer called on seat without the matching capability.");
        return;
    }

    new PointerResource(seatGlobal, id);
}

void SeatGlobal::SeatGlobalPrivate::get_keyboard(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);

    SeatGlobal *seatResource = (SeatGlobal*)wl_resource_get_user_data(resource);

    if(!(seatResource->client()->seat()->capabilities() & LSeat::Keyboard))
    {
        wl_resource_post_error(resource, WL_SEAT_ERROR_MISSING_CAPABILITY, "get_keyboard called on seat without the matching capability.");
        return;
    }

    new KeyboardResource(seatResource, id);
}

void SeatGlobal::SeatGlobalPrivate::get_touch(wl_client *client, wl_resource *resource, UInt32 id)
{
    (void)client;(void)resource;(void)id;

    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    if(!(lClient->seat()->capabilities() & LSeat::Touch))
    {
        wl_resource_post_error(resource, WL_SEAT_ERROR_MISSING_CAPABILITY, "get_touch called on seat without the matching capability.");
        return;
    }
}

#if LOUVRE_SEAT_VERSION >= WL_SEAT_RELEASE_SINCE_VERSION
void SeatGlobal::SeatGlobalPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
