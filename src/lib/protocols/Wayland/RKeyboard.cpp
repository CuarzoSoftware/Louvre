#include <LTime.h>
#include <LSeat.h>
#include <private/LKeyboardPrivate.h>
#include <LClient.h>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <protocols/Wayland/private/RKeyboardPrivate.h>

#include <LCompositor.h>

using namespace Louvre;

static struct wl_keyboard_interface keyboard_implementation =
{
#if LOUVRE_WL_SEAT_VERSION >= 3
    .release = &RKeyboard::RKeyboardPrivate::release
#endif
};

RKeyboard::RKeyboard
(
    GSeat *gSeat,
    Int32 id
)
    :LResource
    (
        gSeat->client(),
        &wl_keyboard_interface,
        gSeat->version(),
        id,
        &keyboard_implementation,
        &RKeyboard::RKeyboardPrivate::resource_destroy
    )
{
    m_imp = new RKeyboardPrivate();
    imp()->gSeat = gSeat;
    LKeyboard *lKeyboard = seat()->keyboard();
    repeatInfo(lKeyboard->repeatRate(), lKeyboard->repeatDelay());
    keymap(lKeyboard->keymapFormat(), lKeyboard->keymapFd(), lKeyboard->keymapSize());
    gSeat->imp()->rKeyboard = this;
}

RKeyboard::~RKeyboard()
{
    if (seatGlobal())
        seatGlobal()->imp()->rKeyboard = nullptr;

    delete m_imp;
}

GSeat *RKeyboard::seatGlobal() const
{
    return imp()->gSeat;
}

const RKeyboard::LastEventSerials &RKeyboard::serials() const
{
    return imp()->serials;
}

bool RKeyboard::keymap(UInt32 format, Int32 fd, UInt32 size)
{
    wl_keyboard_send_keymap(resource(), format, fd, size);
    return true;
}

bool RKeyboard::enter(UInt32 serial, RSurface *rSurface, wl_array *keys)
{
    wl_keyboard_send_enter(resource(), serial, rSurface->resource(), keys);
    return true;
}

bool RKeyboard::leave(UInt32 serial, RSurface *rSurface)
{
    wl_keyboard_send_leave(resource(), serial, rSurface->resource());
    return true;
}

bool RKeyboard::key(UInt32 serial, UInt32 time, UInt32 key, UInt32 state)
{
    wl_keyboard_send_key(resource(), serial, time, key, state);
    return true;
}

bool RKeyboard::modifiers(UInt32 serial, UInt32 modsDepressed, UInt32 modsLatched, UInt32 modsLocked, UInt32 group)
{
    wl_keyboard_send_modifiers(resource(), serial, modsDepressed, modsLatched, modsLocked, group);
    return true;
}

bool RKeyboard::repeatInfo(Int32 rate, Int32 delay)
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
