#include <protocols/Wayland/LWaylandKeyboardResource.h>
#include <LSeat.h>
#include <private/LClientPrivate.h>
#include <protocols/Wayland/LWaylandPointerResource.h>
#include <protocols/Wayland/private/LWaylandSeatGlobalPrivate.h>

using namespace Louvre::Globals;


static struct wl_seat_interface seat_implementation =
{
    .get_pointer = &LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::get_pointer,
    .get_keyboard = &LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::get_keyboard,
    .get_touch = &LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::get_touch,
#if LOUVRE_SEAT_VERSION >= WL_SEAT_RELEASE_SINCE_VERSION
    .release = &LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::release
#endif
};

void LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::bind(wl_client *client, void *compositor, UInt32 version, UInt32 id)
{
    new LWaylandSeatGlobal((LCompositor*)compositor,
                            client,
                            &wl_seat_interface,
                            version,
                            id,
                            &seat_implementation,
                            &LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::resource_destroy);
}

void LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::resource_destroy(wl_resource *resource)
{
    LWaylandSeatGlobal *seatResource = (LWaylandSeatGlobal*)wl_resource_get_user_data(resource);
    delete seatResource;
}

void LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::get_pointer(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);

    LWaylandSeatGlobal *seatGlobal = (LWaylandSeatGlobal*)wl_resource_get_user_data(resource);

    if(!(seatGlobal->client()->seat()->capabilities() & LSeat::Pointer))
    {
        wl_resource_post_error(resource,WL_SEAT_ERROR_MISSING_CAPABILITY,"get_pointer called on seat without the matching capability.");
        return;
    }

    new LWaylandPointerResource(seatGlobal, id);
}

void LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::get_keyboard(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);

    LWaylandSeatGlobal *seatResource = (LWaylandSeatGlobal*)wl_resource_get_user_data(resource);

    if(!(seatResource->client()->seat()->capabilities() & LSeat::Keyboard))
    {
        wl_resource_post_error(resource, WL_SEAT_ERROR_MISSING_CAPABILITY, "get_keyboard called on seat without the matching capability.");
        return;
    }

    new LWaylandKeyboardResource(seatResource, id);
}

void LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::get_touch(wl_client *client, wl_resource *resource, UInt32 id)
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
void LWaylandSeatGlobal::LWaylandSeatGlobalPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
