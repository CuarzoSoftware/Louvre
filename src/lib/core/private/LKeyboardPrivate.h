#ifndef LKEYBOARDPRIVATE_H
#define LKEYBOARDPRIVATE_H

#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <LKeyboard.h>
#include <LKeyboardModifiersEvent.h>
#include <vector>
#include <algorithm>

using namespace Louvre;
using namespace Louvre::Protocols;

struct LKeyboard::Params
{
    /* Add here any required constructor param */
};

LPRIVATE_CLASS(LKeyboard)

    LWeak<LSurface> focus;

    // XKB
    xkb_context *xkbContext         { nullptr };
    xkb_keymap *xkbKeymap           { nullptr };
    xkb_state *xkbKeymapState       { nullptr };
    xkb_rule_names xkbKeymapName;
    Int32 xkbKeymapFd               { -1 };
    Int32 xkbKeymapSize;
    UInt32 keymapFormat;

    LKeyboardModifiersEvent::Modifiers currentModifiersState { 0 };
    LKeyboardModifiersEvent::Modifiers prevModifiersState    { 0 };
    bool modifiersChanged { true };

    Int32 leds[3] { -1, -1, -1 };
    std::vector<UInt32> pressedKeys;

    // Since 4
    Int32 repeatRate    { 32 };
    Int32 repeatDelay   { 500 };

    // Grab
    LWeak<LSurface> grab;
};

#endif // LKEYBOARDPRIVATE_H
