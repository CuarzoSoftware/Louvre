#include <LTime.h>
#include <LWayland.h>
#include <LSeat.h>
#include <private/LKeyboardPrivate.h>
#include <LClient.h>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <protocols/Wayland/private/RKeyboardPrivate.h>

using namespace Louvre;

static struct wl_keyboard_interface keyboard_implementation =
{
#if LOUVRE_SEAT_VERSION >= WL_KEYBOARD_RELEASE_SINCE_VERSION
    .release = &RKeyboard::RKeyboardPrivate::release
#endif
};

RKeyboard::RKeyboard(GSeat *seatGlobal, Int32 id) :
    LResource(seatGlobal->client(),
              &wl_keyboard_interface,
              seatGlobal->version(),
              id,
              &keyboard_implementation,
              &RKeyboard::RKeyboardPrivate::resource_destroy)
{
    m_imp = new RKeyboardPrivate();
    imp()->seatGlobal = seatGlobal;
    LKeyboard *lKeyboard = client()->seat()->keyboard();
    sendRepeatInfo(lKeyboard->repeatRate(), lKeyboard->repeatDelay());
    sendKeymap(lKeyboard->keymapFd(), lKeyboard->keymapSize());
    seatGlobal->imp()->keyboardResource = this;
}

RKeyboard::~RKeyboard()
{
    if (seatGlobal())
        seatGlobal()->imp()->keyboardResource = nullptr;
}

void RKeyboard::sendRepeatInfo(Int32 rate, Int32 delay)
{
#if LOUVRE_SEAT_VERSION >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
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
    imp()->serials.leave = LWayland::nextSerial();
    wl_keyboard_send_leave(resource(), serials().leave, surface->surfaceResource()->resource());
}

void RKeyboard::sendEnter(LSurface *surface)
{
    imp()->serials.enter = LWayland::nextSerial();
    wl_keyboard_send_enter(resource(), serials().enter, surface->surfaceResource()->resource(), &client()->seat()->keyboard()->imp()->keys);
}

void RKeyboard::sendModifiers(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    imp()->serials.modifiers = LWayland::nextSerial();
    wl_keyboard_send_modifiers(resource(), serials().modifiers, depressed, latched, locked, group);
}

void RKeyboard::sendKey(UInt32 key, UInt32 state)
{
    imp()->serials.key = LWayland::nextSerial();
    wl_keyboard_send_key(resource(), serials().key, LTime::ms(), key, state);
}

GSeat *RKeyboard::seatGlobal() const
{
    return imp()->seatGlobal;
}

const RKeyboard::LastEventSerials &RKeyboard::serials() const
{
    return imp()->serials;
}
