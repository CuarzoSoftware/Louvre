#include <LTime.h>
#include <LWayland.h>
#include <LSeat.h>
#include <private/LKeyboardPrivate.h>
#include <LClient.h>
#include <protocols/Wayland/private/LWaylandSeatGlobalPrivate.h>
#include <protocols/Wayland/private/LWaylandKeyboardResourcePrivate.h>

using namespace Louvre;

static struct wl_keyboard_interface keyboard_implementation =
{
#if LOUVRE_SEAT_VERSION >= WL_KEYBOARD_RELEASE_SINCE_VERSION
    .release = &LWaylandKeyboardResource::LWaylandKeyboardResourcePrivate::release
#endif
};

LWaylandKeyboardResource::LWaylandKeyboardResource(LWaylandSeatGlobal *seatGlobal, Int32 id) :
    LResource(seatGlobal->client(),
              &wl_keyboard_interface,
              seatGlobal->version(),
              id,
              &keyboard_implementation,
              &LWaylandKeyboardResource::LWaylandKeyboardResourcePrivate::resource_destroy)
{
    m_imp = new LWaylandKeyboardResourcePrivate();
    imp()->seatGlobal = seatGlobal;
    LKeyboard *lKeyboard = client()->seat()->keyboard();
    sendRepeatInfo(lKeyboard->repeatRate(), lKeyboard->repeatDelay());
    sendKeymap(lKeyboard->keymapFd(), lKeyboard->keymapSize());
    seatGlobal->imp()->keyboardResources.push_back(this);
    imp()->seatLink = std::prev(seatGlobal->imp()->keyboardResources.end());
}

LWaylandKeyboardResource::~LWaylandKeyboardResource()
{
    if(seatGlobal())
        seatGlobal()->imp()->keyboardResources.erase(imp()->seatLink);
}

void LWaylandKeyboardResource::sendRepeatInfo(Int32 rate, Int32 delay)
{
#if LOUVRE_SEAT_VERSION >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    if(version() >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
        wl_keyboard_send_repeat_info(resource(), rate, delay);
#endif
}

void LWaylandKeyboardResource::sendKeymap(Int32 fd, UInt32 size)
{
    // TODO: CHECK v7 PRIVATE_MAP
    wl_keyboard_send_keymap(resource(), WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, size);
}

void LWaylandKeyboardResource::sendLeave(LSurface *surface)
{
    seatGlobal()->imp()->keyboardSerials.leave = LWayland::nextSerial();
    wl_keyboard_send_leave(resource(), seatGlobal()->keyboardSerials().leave, surface->resource());
}

void LWaylandKeyboardResource::sendEnter(LSurface *surface)
{
    seatGlobal()->imp()->keyboardSerials.enter = LWayland::nextSerial();
    wl_keyboard_send_enter(resource(), seatGlobal()->keyboardSerials().enter, surface->resource(), &client()->seat()->keyboard()->imp()->keys);
}

void LWaylandKeyboardResource::sendModifiers(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    seatGlobal()->imp()->keyboardSerials.modifiers = LWayland::nextSerial();
    wl_keyboard_send_modifiers(resource(), seatGlobal()->keyboardSerials().modifiers, depressed, latched, locked, group);
}

void LWaylandKeyboardResource::sendKey(UInt32 key, UInt32 state)
{
    seatGlobal()->imp()->keyboardSerials.key = LWayland::nextSerial();
    wl_keyboard_send_key(resource(), seatGlobal()->keyboardSerials().key, LTime::ms(), key, state);
}

LWaylandSeatGlobal *LWaylandKeyboardResource::seatGlobal() const
{
    return imp()->seatGlobal;
}

LWaylandKeyboardResource::LWaylandKeyboardResourcePrivate *LWaylandKeyboardResource::imp() const
{
    return m_imp;
}


