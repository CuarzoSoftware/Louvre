#include <private/LKeyboardPrivate.h>
#include <LKeyboardKeyEvent.h>

using namespace Louvre;

void LKeyboardKeyEvent::notify()
{
    LKeyboard::LKeyboardPrivate &keyboard { *seat()->keyboard()->imp() };

    if (keyboard.xkbKeymapState)
    {
        xkb_state_update_key(keyboard.xkbKeymapState,
                             keyCode()+8,
                             (xkb_key_direction)state());

        keyboard.currentModifiersState.depressed = xkb_state_serialize_mods(keyboard.xkbKeymapState, XKB_STATE_MODS_DEPRESSED);
        keyboard.currentModifiersState.latched = xkb_state_serialize_mods(keyboard.xkbKeymapState, XKB_STATE_MODS_LATCHED);
        keyboard.currentModifiersState.locked = xkb_state_serialize_mods(keyboard.xkbKeymapState, XKB_STATE_MODS_LOCKED);
        keyboard.currentModifiersState.group = xkb_state_serialize_layout(keyboard.xkbKeymapState, XKB_STATE_LAYOUT_EFFECTIVE);

        if (keyboard.currentModifiersState.depressed != keyboard.prevModifiersState.depressed ||
            keyboard.currentModifiersState.latched != keyboard.prevModifiersState.latched ||
            keyboard.currentModifiersState.locked != keyboard.prevModifiersState.locked ||
            keyboard.currentModifiersState.group != keyboard.prevModifiersState.group)
        {
            keyboard.modifiersChanged = true;
            keyboard.prevModifiersState = keyboard.currentModifiersState;
        }
    }

    if (state() == State::Pressed)
        keyboard.pressedKeys.push_back(keyCode());
    else
        LVectorRemoveOneUnordered(keyboard.pressedKeys, keyCode());


    // CTRL + ALT + (F1, F2, ..., F10) : Switch TTY.
    if (seat()->imp()->libseatHandle &&
        keyCode() >= KEY_F1 && keyCode() <= KEY_F10 &&
        seat()->keyboard()->isKeyCodePressed(KEY_LEFTCTRL) &&
        seat()->keyboard()->isKeyCodePressed(KEY_LEFTALT)
        )
        seat()->setTTY(keyCode() - KEY_F1 + 1);

    if (compositor()->state() == LCompositor::Initialized)
        seat()->keyboard()->keyEvent(*this);

    keyboard.modifiersChanged = false;
}
