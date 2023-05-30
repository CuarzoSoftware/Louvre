#ifndef LKEYBOARDPRIVATE_H
#define LKEYBOARDPRIVATE_H

#include <LKeyboard.h>

using namespace Louvre;

struct LKeyboard::Params
{
    /* Add here any required constructor param */
};

LPRIVATE_CLASS(LKeyboard)

    LSurface *keyboardFocusSurface = nullptr;

#if LOUVRE_SEAT_VERSION >= 4
    Int32 repeatRate = 32;
    Int32 repeatDelay = 500;
#endif

    wl_array keys;

    // XKB
    xkb_context *xkbContext = nullptr;
    xkb_keymap *xkbKeymap = nullptr;
    xkb_state *xkbKeymapState = nullptr;
    xkb_rule_names xkbKeymapName;
    Int32 xkbKeymapSize;
    Int32 xkbKeymapFd = -1;

    void updateModifiers();

    KeyboardModifiersState modifiersState;
};

#endif // LKEYBOARDPRIVATE_H
