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
#if LOUVRE_WL_SEAT_VERSION >= WL_KEYBOARD_RELEASE_SINCE_VERSION
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
    sendRepeatInfo(lKeyboard->repeatRate(), lKeyboard->repeatDelay());
    sendKeymap(lKeyboard->keymapFd(), lKeyboard->keymapSize());
    gSeat->imp()->keyboardResource = this;
}

RKeyboard::~RKeyboard()
{
    if (seatGlobal())
        seatGlobal()->imp()->keyboardResource = nullptr;
}

void RKeyboard::sendRepeatInfo(Int32 rate, Int32 delay)
{
#if LOUVRE_WL_SEAT_VERSION >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    if (version() >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
        wl_keyboard_send_repeat_info(resource(), rate, delay);
#endif
}

void RKeyboard::sendKeymap(Int32 fd, UInt32 size)
{
    // TODO: CHECK v7 PRIVATE_MAP
    wl_keyboard_send_keymap(resource(), WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, size);
}

void RKeyboard::sendLeave(LSurface *surface)
{
    imp()->serials.leave = LCompositor::nextSerial();
    wl_keyboard_send_leave(resource(), serials().leave, surface->surfaceResource()->resource());
}

void RKeyboard::sendEnter(LSurface *surface)
{
    imp()->serials.enter = LCompositor::nextSerial();
    wl_array keys;

    wl_array_init(&keys);

    for (UInt32 key : seat()->keyboard()->pressedKeys())
    {
        UInt32 *p = (UInt32*)wl_array_add(&keys, sizeof(UInt32));
        *p = key;
    }

    wl_keyboard_send_enter(resource(), serials().enter, surface->surfaceResource()->resource(), &keys);
    wl_array_release(&keys);
}

void RKeyboard::sendModifiers(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    imp()->serials.modifiers = LCompositor::nextSerial();
    wl_keyboard_send_modifiers(resource(), serials().modifiers, depressed, latched, locked, group);
}

void RKeyboard::sendKey(UInt32 key, UInt32 state)
{
    imp()->serials.key = LCompositor::nextSerial();
    wl_keyboard_send_key(resource(), serials().key, LTime::ms(), key, state);
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
