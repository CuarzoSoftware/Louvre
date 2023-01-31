#ifndef LKEYBOARDPRIVATE_H
#define LKEYBOARDPRIVATE_H

#include <LKeyboard.h>

struct Louvre::LKeyboard::Params
{
    LSeat *seat;
};

class Louvre::LKeyboard::LKeyboardPrivate
{
public:
    LKeyboardPrivate()                                      = default;
    ~LKeyboardPrivate()                                     = default;

    LKeyboardPrivate(const LKeyboardPrivate&)               = delete;
    LKeyboardPrivate &operator=(const LKeyboardPrivate&)    = delete;

    LKeyboard *keyboard                                     = nullptr;

    LSeat *seat                                             = nullptr;

    // Wayland
    LSurface *keyboardFocusSurface                          = nullptr;

#if LOUVRE_SEAT_VERSION >= 4
    Int32 repeatRate                                        = 32;
    Int32 repeatDelay                                       = 500;
#endif

    wl_array keys;

    // XKB
    xkb_context *xkbContext                                 = nullptr;
    xkb_keymap *xkbKeymap                                   = nullptr;
    xkb_state *xkbKeymapState                               = nullptr;
    xkb_rule_names xkbKeymapName;
    Int32 xkbKeymapSize;
    Int32 xkbKeymapFd                                       = -1;

    void updateModifiers();

    KeyboardModifiersState modifiersState;

};


#endif // LKEYBOARDPRIVATE_H
