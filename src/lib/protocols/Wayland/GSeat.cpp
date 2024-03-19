#include <protocols/Wayland/private/RKeyboardPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/Wayland/private/RTouchPrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LSeat.h>

static const struct wl_seat_interface imp
{
    .get_pointer = &GSeat::get_pointer,
    .get_keyboard = &GSeat::get_keyboard,
    .get_touch = &GSeat::get_touch,
#if LOUVRE_WL_SEAT_VERSION >= 5
    .release = &GSeat::release
#endif
};

GSeat::GSeat(
    wl_client *client,
    Int32 version,
    UInt32 id) noexcept
    :LResource
    (
        client,
        &wl_seat_interface,
        version,
        id,
        &imp
    )
{
    this->client()->imp()->seatGlobals.push_back(this);
    capabilities(seat()->inputCapabilities());
    name(seat()->name());
}

GSeat::~GSeat() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->seatGlobals, this);

    while (!keyboardRes().empty())
    {
        keyboardRes().back()->imp()->gSeat = nullptr;
        m_keyboardRes.pop_back();
    }

    while (!pointerRes().empty())
    {
        pointerRes().back()->imp()->gSeat = nullptr;
        m_pointerRes.pop_back();
    }

    while (!touchRes().empty())
    {
        touchRes().back()->imp()->gSeat = nullptr;
        m_touchRes.pop_back();
    }
}

/******************** REQUESTS ********************/

void GSeat::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GSeat(client, version, id);
}

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

bool GSeat::capabilities(UInt32 capabilities) noexcept
{
    wl_seat_send_capabilities(resource(), capabilities);
    return true;
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
