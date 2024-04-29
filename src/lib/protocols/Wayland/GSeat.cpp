#include <protocols/Wayland/RTouch.h>
#include <protocols/Wayland/RPointer.h>
#include <protocols/Wayland/RKeyboard.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LUtils.h>

using namespace Louvre::Protocols::Wayland;

static const struct wl_seat_interface imp
{
    .get_pointer = &GSeat::get_pointer,
    .get_keyboard = &GSeat::get_keyboard,
    .get_touch = &GSeat::get_touch,
#if LOUVRE_WL_SEAT_VERSION >= 5
    .release = &GSeat::release
#endif
};

void GSeat::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GSeat(client, version, id);
}

Int32 GSeat::maxVersion() noexcept
{
    return LOUVRE_WL_SEAT_VERSION;
}

const wl_interface *GSeat::interface() noexcept
{
    return &wl_seat_interface;
}

GSeat::GSeat(
    wl_client *client,
    Int32 version,
    UInt32 id) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->seatGlobals.push_back(this);
    capabilities(7);
    name(seat()->name());
}

GSeat::~GSeat() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->seatGlobals, this);
}

/******************** REQUESTS ********************/

void GSeat::get_pointer(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    // WL_SEAT_ERROR_MISSING_CAPABILITY check not needed
    new RPointer(static_cast<GSeat*>(wl_resource_get_user_data(resource)), id);
}

void GSeat::get_keyboard(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    // WL_SEAT_ERROR_MISSING_CAPABILITY check not needed
    new RKeyboard(static_cast<GSeat*>(wl_resource_get_user_data(resource)), id);
}

void GSeat::get_touch(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    // WL_SEAT_ERROR_MISSING_CAPABILITY check not needed
    new RTouch(static_cast<GSeat*>(wl_resource_get_user_data(resource)), id);
}

#if LOUVRE_WL_SEAT_VERSION >= 5
void GSeat::release(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
#endif

/******************** EVENTS ********************/

void GSeat::capabilities(UInt32 capabilities) noexcept
{
    wl_seat_send_capabilities(resource(), capabilities);
}

bool GSeat::name(const char *name) noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 2
    if (version() >= 2)
    {
        wl_seat_send_name(resource(), name);
        return true;
    }
#endif
    L_UNUSED(name);
    return false;
}
