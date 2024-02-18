#include <protocols/Wayland/private/RKeyboardPrivate.h>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LTime.h>

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
    ),
    LPRIVATE_INIT_UNIQUE(RKeyboard)
{
    imp()->gSeat = gSeat;
    const LKeyboard *lKeyboard { seat()->keyboard() };
    repeatInfo(lKeyboard->repeatRate(), lKeyboard->repeatDelay());
    keymap(lKeyboard->keymapFormat(), lKeyboard->keymapFd(), lKeyboard->keymapSize());
    gSeat->imp()->rKeyboards.push_back(this);
}

RKeyboard::~RKeyboard()
{
    if (seat()->keyboard()->grabbingKeyboardResource() == this)
        seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);

    if (seatGlobal())
        LVectorRemoveOneUnordered(seatGlobal()->imp()->rKeyboards, this);
}

GSeat *RKeyboard::seatGlobal() const
{
    return imp()->gSeat;
}

bool RKeyboard::keymap(UInt32 format, Int32 fd, UInt32 size)
{
    wl_keyboard_send_keymap(resource(), format, fd, size);
    return true;
}

bool RKeyboard::enter(const LKeyboardEnterEvent &event, RSurface *rSurface, wl_array *keys)
{
    auto &clientEvent = client()->imp()->events.keyboard.enter;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_keyboard_send_enter(resource(), event.serial(), rSurface->resource(), keys);
    return true;
}

bool RKeyboard::leave(const LKeyboardLeaveEvent &event, RSurface *rSurface)
{
    auto &clientEvent = client()->imp()->events.keyboard.leave;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_keyboard_send_leave(resource(), event.serial(), rSurface->resource());
    return true;
}

bool RKeyboard::key(const LKeyboardKeyEvent &event)
{
    auto &clientEvent = client()->imp()->events.keyboard.key;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_keyboard_send_key(resource(), event.serial(), event.ms(), event.keyCode(), event.state());
    return true;
}

bool RKeyboard::modifiers(const LKeyboardModifiersEvent &event)
{
    auto &clientEvent = client()->imp()->events.keyboard.modifiers;

    if (clientEvent.serial() != event.serial())
        clientEvent = event;

    wl_keyboard_send_modifiers(resource(),
                               event.serial(),
                               event.modifiers().depressed,
                               event.modifiers().latched,
                               event.modifiers().locked,
                               event.modifiers().group);
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
