#ifndef LKEYBOARDPRIVATE_H
#define LKEYBOARDPRIVATE_H

#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <LKeyboard.h>
#include <vector>
#include <algorithm>

using namespace Louvre;
using namespace Louvre::Protocols;

struct LKeyboard::Params
{
    /* Add here any required constructor param */
};

LPRIVATE_CLASS(LKeyboard)

    LSurface *keyboardFocusSurface = nullptr;

    // XKB
    xkb_context *xkbContext = nullptr;
    xkb_keymap *xkbKeymap = nullptr;
    xkb_state *xkbKeymapState = nullptr;
    xkb_rule_names xkbKeymapName;
    Int32 xkbKeymapSize;
    Int32 xkbKeymapFd = -1;
    UInt32 keymapFormat;

    KeyboardModifiersState modifiersState;

    std::vector<UInt32>pressedKeys;

    // Since 4
    Int32 repeatRate = 32;
    Int32 repeatDelay = 500;

    // Grab
    LSurface *grabbingSurface = nullptr;
    Wayland::RKeyboard *grabbingKeyboardResource = nullptr;

    inline void backendKeyEvent(UInt32 keyCode, KeyState keyState)
    {
        if (xkbKeymapState)
            xkb_state_update_key(xkbKeymapState,
                                 keyCode+8,
                                 (xkb_key_direction)keyState);

        if (keyState == LKeyboard::Pressed)
            pressedKeys.push_back(keyCode);
        else
            LVectorRemoveOneUnordered(pressedKeys, keyCode);

        seat()->keyboard()->keyEvent(keyCode, keyState);
        updateModifiers();

        // CTRL + ALT + (F1, F2, ..., F10) : Switch TTY.
        if (seat()->imp()->libseatHandle &&
            keyCode >= KEY_F1 && keyCode <= KEY_F10 &&
            seat()->keyboard()->isKeyCodePressed(KEY_LEFTCTRL) &&
            seat()->keyboard()->isKeyCodePressed(KEY_LEFTALT)
            )
        seat()->setTTY(keyCode - KEY_F1 + 1);
    }

    inline void updateModifiers()
    {
        if (xkbKeymapState)
        {
            modifiersState.depressed = xkb_state_serialize_mods(xkbKeymapState, XKB_STATE_MODS_DEPRESSED);
            modifiersState.latched = xkb_state_serialize_mods(xkbKeymapState, XKB_STATE_MODS_LATCHED);
            modifiersState.locked = xkb_state_serialize_mods(xkbKeymapState, XKB_STATE_MODS_LOCKED);
            modifiersState.group = xkb_state_serialize_layout(xkbKeymapState, XKB_STATE_LAYOUT_EFFECTIVE);
        }
        seat()->keyboard()->keyModifiersEvent(modifiersState.depressed,
                                              modifiersState.latched,
                                              modifiersState.locked,
                                              modifiersState.group);
    }
};

#endif // LKEYBOARDPRIVATE_H
