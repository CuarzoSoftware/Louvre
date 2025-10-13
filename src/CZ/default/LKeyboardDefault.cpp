#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/LLauncher.h>
#include <CZ/Core/Events/CZKeyboardKeyEvent.h>
#include <CZ/Core/CZKeymap.h>
#include <unistd.h>

using namespace CZ;

//! [keyEvent]
void LKeyboard::keyEvent(const CZKeyboardKeyEvent &event)
{
    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };

    sendKeyEvent(event);

    const bool L_CTRL      { isKeyCodePressed(KEY_LEFTCTRL)  };
    const bool R_CTRL      { isKeyCodePressed(KEY_RIGHTCTRL) };
    const bool L_SHIFT     { isKeyCodePressed(KEY_LEFTSHIFT) };
    const bool L_ALT       { isKeyCodePressed(KEY_LEFTALT)   };
    const bool mods        { L_ALT || L_SHIFT || L_CTRL || R_CTRL };
    const xkb_keysym_t sym { keySymbol(event.code) };

    if (!event.isPressed)
    {
        if (event.code == KEY_ESC && L_CTRL && L_SHIFT)
        {
            compositor()->finish();
            return;
        }
        else if (L_CTRL && !L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::Copy);
        else if (!L_CTRL && L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::Move);
        else if (!L_CTRL && !L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::NoAction);

        if (L_SHIFT && L_CTRL)
        {
            if (event.code == KEY_T && cursor()->output())
            {
                if (cursor()->output()->transform() == CZTransform::Flipped270)
                    cursor()->output()->setTransform(CZTransform::Normal);
                else
                    cursor()->output()->setTransform((CZTransform)((Int32)cursor()->output()->transform() + 1));
            }
        }

        if (sessionLocked)
            return;

        if (event.code == KEY_F1 && !mods)
        {
            if (LLauncher::launch("kitty") < 0)
                if (LLauncher::launch("foot") < 0)
                    LLauncher::launch("gnome-terminal");
        }
        else if (L_CTRL && (sym == XKB_KEY_q || sym == XKB_KEY_Q))
        {
            if (focus())
                focus()->client()->destroyLater();
        }
        else if (L_CTRL && (sym == XKB_KEY_m || sym == XKB_KEY_M))
        {
            if (focus() && focus()->toplevel() && !focus()->toplevel()->isFullscreen())
                focus()->toplevel()->setMinimized(true);
        }
    }

    // Key pressed
    else
    {
        // CTRL sets Copy as the preferred action in drag & drop session
        if (L_CTRL)
            seat()->dnd()->setPreferredAction(LDND::Copy);

        // SHIFT sets Move as the preferred action in drag & drop session
        else if (L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::Move);
    }
}
//! [keyEvent]

//! [focusChanged]
void LKeyboard::focusChanged()
{
    /* No default implementation. */
}
//! [focusChanged]


void LKeyboard::handleTTYSwitchShortcut(const CZKeyboardKeyEvent &e) noexcept
{
    if (seat()->libseatHandle()
        && isKeyCodePressed(KEY_LEFTALT)
        && (seat()->keyboard()->isKeyCodePressed(KEY_LEFTCTRL) || seat()->keyboard()->isKeyCodePressed(KEY_RIGHTCTRL))
        && e.code >= KEY_F1 && e.code <= KEY_F10)
    {
        seat()->setTTY(e.code - KEY_F1 + 1);
        return;
    }

    e.ignore();
}

void LKeyboard::updateKeymapState(const CZKeyboardKeyEvent &e) noexcept
{
    if (keymap())
    {
        keymap()->feed(e);

        // Update backend LEDs
        const xkb_led_index_t idx[] {
            xkb_keymap_led_get_index(keymap()->keymap(), XKB_LED_NAME_NUM),
            xkb_keymap_led_get_index(keymap()->keymap(), XKB_LED_NAME_CAPS),
            xkb_keymap_led_get_index(keymap()->keymap(), XKB_LED_NAME_SCROLL)
        };

        UInt32 ledsMask { 0 };
        for (UInt32 i = 0; i < 3; i++)
            if (xkb_state_led_index_is_active(keymap()->state(), idx[i]) == 1)
                ledsMask |= 1 << i;

        compositor()->backend()->inputSetLeds(ledsMask);
    }
}

bool LKeyboard::event(const CZEvent &e) noexcept
{
    if (e.type() == CZEvent::Type::KeyboardKey)
    {
        const auto &keyEv { (const CZKeyboardKeyEvent&)e };

        handleTTYSwitchShortcut(keyEv);
        if (keyEv.isAccepted())
            return true;

        updateKeymapState(keyEv);
        keyEvent(keyEv);
    }

    return LFactoryObject::event(e);
}
