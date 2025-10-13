#include "Core/CZCore.h"
#include <CZ/Louvre/Protocols/Wayland/RKeyboard.h>
#include <CZ/Louvre/Protocols/Wayland/RDataDevice.h>
#include <CZ/Louvre/Protocols/Wayland/RDataOffer.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Private/LKeyboardPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Seat/LClipboard.h>
#include <CZ/Louvre/LObject.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/CZKeymap.h>
#include <CZ/Core/CZTime.h>
#include <cassert>

using namespace CZ::Protocols::Wayland;

LKeyboard::LKeyboard(const void *params) noexcept : LFactoryObject(FactoryObjectType), LPRIVATE_INIT_UNIQUE(LKeyboard)
{
    assert(params != nullptr && "Invalid parameter passed to LKeyboard constructor.");
    LKeyboard **ptr { (LKeyboard**) params };
    assert(*ptr == nullptr && *ptr == seat()->keyboard() && "Only a single LKeyboard instance can exist.");
    *ptr = this;

    log = LLog.newWithContext("LKeyboard");

    auto core { CZCore::Get() };
    core->onKeymapChanged.subscribe(this, [](){

        auto core { CZCore::Get() };

        if (auto keymap = core->keymap())
        {
            const auto format { keymap ? WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 : WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP };
            const auto fd { keymap ? keymap->fd() : -1 };
            const auto size { keymap ? keymap->size() : 0 };

            for (auto client : compositor()->clients())
                for (auto gSeat : client->seatGlobals())
                    for (auto rKeyboard : gSeat->keyboardRes())
                        rKeyboard->keymap(format, fd, size);
        }
    });

    imp()->focus.setOnDestroyCallback([this](auto)
    {
        focusChanged();
    });
}

LKeyboard::~LKeyboard() noexcept
{
    notifyDestruction();
}

void LKeyboard::setGrab(LSurface *surface) noexcept
{
    if (imp()->grab == surface)
        return;

    if (imp()->focus == surface)
        imp()->focus.reset();

    imp()->grab.reset();
    setFocus(surface);
    imp()->grab.reset(surface);
}

LSurface *LKeyboard::grab() const noexcept
{
    return imp()->grab;
}

std::shared_ptr<CZKeymap> LKeyboard::keymap() const noexcept
{
    return CZCore::Get()->keymap();
}

bool LKeyboard::setKeymap(const char *rules, const char *model, const char *layout, const char *variant, const char *options) noexcept
{    
    xkb_rule_names names {
        .rules = rules,
        .model = model,
        .layout = layout,
        .variant = variant,
        .options = options
    };

    auto keymap { CZKeymap::MakeServer(names) };

    if (!keymap)
    {
        log(CZError, CZLN, "Failed to create keymap");
        return false;
    }

    CZCore::Get()->setKeymap(keymap);
    return true;
}

LSurface *LKeyboard::focus() const noexcept
{
    return imp()->focus;
}

void LKeyboard::setFocus(LSurface *surface)
{
    if (imp()->grab)
        return;

    CZWeak<LSurface> prev { focus() };

    if (surface)
    {
        if (focus() == surface)
            return;

        if (focus())
        {
            CZKeyboardLeaveEvent leaveEvent;

            for (auto gSeat : focus()->client()->seatGlobals())
                for (auto rKeyboard : gSeat->keyboardRes())
                    rKeyboard->leave(leaveEvent, focus()->surfaceResource());
        }

        const bool clientChanged { !seat()->clipboard()->m_dataOffer || seat()->clipboard()->m_dataOffer->client() != surface->client()};

        if (clientChanged && seat()->clipboard()->m_dataOffer && seat()->clipboard()->m_dataOffer->dataDeviceRes())
            seat()->clipboard()->m_dataOffer->dataDeviceRes()->selection(nullptr);

        imp()->focus.reset();

        // Pack currently pressed keys
        wl_array keys;
        wl_array_init(&keys);

        for (UInt32 key : keymap()->pressedKeys())
            *(UInt32*)wl_array_add(&keys, sizeof(UInt32)) = key;

        CZKeyboardEnterEvent enterEvent;
        CZKeyboardModifiersEvent modifiersEvent {};
        modifiersEvent.modifiers = keymap()->modifiers();

        bool selectionSent { false };

        for (auto gSeat : surface->client()->seatGlobals())
        {
            if (clientChanged && !selectionSent && gSeat->dataDeviceRes())
            {
                gSeat->dataDeviceRes()->createOffer(RDataSource::Clipboard);
                selectionSent = true;
            }

            for (auto rKeyboard : gSeat->keyboardRes())
            {
                imp()->focus.reset(surface);
                rKeyboard->enter(enterEvent, surface->surfaceResource(), &keys);
                rKeyboard->modifiers(modifiersEvent);
            }
        }

        wl_array_release(&keys);
    }
    else
    {
        // Remove focus from current surface
        if (focus())
        {
            CZKeyboardLeaveEvent leaveEvent;

            for (auto gSeat : focus()->client()->seatGlobals())
                for (auto rKeyboard : gSeat->keyboardRes())
                    rKeyboard->leave(leaveEvent, focus()->surfaceResource());
        }

        if (seat()->clipboard()->m_dataOffer && seat()->clipboard()->m_dataOffer->dataDeviceRes())
            seat()->clipboard()->m_dataOffer->dataDeviceRes()->selection(nullptr);

        imp()->focus.reset();
    }

    if (prev.get() != focus())
        focusChanged();
}

void LKeyboard::sendKeyEvent(const CZKeyboardKeyEvent &event) noexcept
{
    if (!focus())
        return;

    CZKeyboardModifiersEvent modifiersEvent {};
    modifiersEvent.modifiers = keymap()->modifiers();

    for (auto *gSeat : focus()->client()->seatGlobals())
    {
        for (auto *rKeyboard : gSeat->keyboardRes())
        {
            rKeyboard->key(event);

            if (rKeyboard->lastSentModifiers() != modifiersEvent.modifiers)
                rKeyboard->modifiers(modifiersEvent);
        }
    }
}

xkb_keysym_t LKeyboard::keySymbol(UInt32 keyCode) noexcept
{
    if (!keymap())
        return keyCode;

    return xkb_state_key_get_one_sym(keymap()->state(), keyCode+8);
}

bool LKeyboard::isModActive(const char *name, xkb_state_component type) const noexcept
{
    if (!keymap())
        return false;

    return xkb_state_mod_name_is_active(keymap()->state(), name, type) == 1;
}

bool LKeyboard::isKeyCodePressed(UInt32 keyCode) const noexcept
{
    if (!keymap())
        return false;

    return keymap()->pressedKeys().contains(keyCode);
}

bool LKeyboard::isKeySymbolPressed(xkb_keysym_t keySymbol) const noexcept
{
    if (keymap())
        for (UInt32 key : keymap()->pressedKeys())
            if (xkb_state_key_get_one_sym(keymap()->state(), key + 8) == keySymbol)
                return true;

    return false;
}

// Since 4

Int32 LKeyboard::repeatRate() const noexcept
{
    return imp()->repeatRate;
}

Int32 LKeyboard::repeatDelay() const noexcept
{
    return imp()->repeatDelay;
}

void LKeyboard::setRepeatInfo(Int32 rate, Int32 msDelay) noexcept
{
    imp()->repeatRate = rate < 0 ? 0 : rate;
    imp()->repeatDelay = msDelay < 0 ? 0 : msDelay;

    for (auto client : compositor()->clients())
        for (auto gSeat : client->seatGlobals())
            for (auto rKeyboard : gSeat->keyboardRes())
                rKeyboard->repeatInfo(rate, msDelay);
}
