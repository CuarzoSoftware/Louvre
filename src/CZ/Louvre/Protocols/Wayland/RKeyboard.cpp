#include <CZ/Louvre/Protocols/Wayland/RKeyboard.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Private/LKeyboardPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LTime.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::Wayland;

static const  struct wl_keyboard_interface imp
{
#if LOUVRE_WL_SEAT_VERSION >= 3
    .release = &RKeyboard::release
#endif
};

RKeyboard::RKeyboard
(
    GSeat *seatRes,
    Int32 id
) noexcept
    :LResource
    (
        seatRes->client(),
        &wl_keyboard_interface,
        seatRes->version(),
        id,
        &imp
    ),
    m_seatRes(seatRes)
{
    repeatInfo(seat()->keyboard()->repeatRate(), seat()->keyboard()->repeatDelay());
    keymap(seat()->keyboard()->keymapFormat(), seat()->keyboard()->keymapFd(), seat()->keyboard()->keymapSize());
    seatRes->m_keyboardRes.emplace_back(this);
}

RKeyboard::~RKeyboard() noexcept
{
    if (seatRes())
        LVectorRemoveOneUnordered(seatRes()->m_keyboardRes, this);
}

/******************** REQUESTS********************/

#if LOUVRE_WL_SEAT_VERSION >= 3
void RKeyboard::release(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
#endif

/******************** EVENTS ********************/


void RKeyboard::keymap(UInt32 format, Int32 fd, UInt32 size) noexcept
{
    wl_keyboard_send_keymap(resource(), format, fd, size);
}

void RKeyboard::enter(const LKeyboardEnterEvent &event, RSurface *surfaceRes, wl_array *keys) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.keyboard.enter };

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_keyboard_send_enter(resource(), event.serial(), surfaceRes->resource(), keys);
}

void RKeyboard::leave(const LKeyboardLeaveEvent &event, RSurface *surfaceRes) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.keyboard.leave };

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_keyboard_send_leave(resource(), event.serial(), surfaceRes->resource());
}

void RKeyboard::key(const LKeyboardKeyEvent &event) noexcept
{
    auto &clientEvents { client()->imp()->eventHistory.keyboard };

    if (clientEvents.key[clientEvents.keyIndex].serial() != event.serial())
    {
        if (clientEvents.keyIndex == 4)
            clientEvents.keyIndex = 0;
        else
            clientEvents.keyIndex++;

        clientEvents.key[clientEvents.keyIndex] = event;
    }

    wl_keyboard_send_key(resource(), event.serial(), event.ms(), event.keyCode(), event.state());
}

void RKeyboard::modifiers(const LKeyboardModifiersEvent &event) noexcept
{
    auto &clientEvent { client()->imp()->eventHistory.keyboard.modifiers };

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_keyboard_send_modifiers(resource(),
                               event.serial(),
                               event.modifiers().depressed,
                               event.modifiers().latched,
                               event.modifiers().locked,
                               event.modifiers().group);
}

bool RKeyboard::repeatInfo(Int32 rate, Int32 delay) noexcept
{
#if LOUVRE_WL_SEAT_VERSION >= 4
    if (version() >= 4)
    {
        wl_keyboard_send_repeat_info(resource(), rate, delay);
        return true;
    }
#endif
    L_UNUSED(rate);
    L_UNUSED(delay);
    return false;
}
