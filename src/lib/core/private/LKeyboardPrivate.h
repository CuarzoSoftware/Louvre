#ifndef LKEYBOARDPRIVATE_H
#define LKEYBOARDPRIVATE_H

#include <LKeyboard.h>
#include <vector>

using namespace Louvre;

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

    void preKeyEvent(UInt32 keyCode, UInt32 keyState);
    void updateModifiers();

    KeyboardModifiersState modifiersState;

    std::list<UInt32>pressedKeys;

    // Since 4
    Int32 repeatRate = 32;
    Int32 repeatDelay = 500;
};

#endif // LKEYBOARDPRIVATE_H
