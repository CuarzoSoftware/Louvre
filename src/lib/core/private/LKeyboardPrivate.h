#ifndef LKEYBOARDPRIVATE_H
#define LKEYBOARDPRIVATE_H

#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <LKeyboard.h>
#include <LKeyboardModifiersEvent.h>
#include <vector>

using namespace Louvre;
using namespace Louvre::Protocols;

LPRIVATE_CLASS(LKeyboard)
    std::vector<UInt32> pressedKeys;
    LWeak<LSurface> focus;
    LKeyboardModifiersEvent::Modifiers currentModifiersState { 0 };
    LKeyboardModifiersEvent::Modifiers prevModifiersState { 0 };
    LWeak<LSurface> grab;
    xkb_rule_names xkbKeymapName;
    xkb_context *xkbContext { nullptr };
    xkb_keymap *xkbKeymap { nullptr };
    xkb_state *xkbKeymapState { nullptr };
    Int32 xkbKeymapFd { -1 };
    Int32 xkbKeymapSize;
    UInt32 keymapFormat;
    Int32 leds[3] { -1, -1, -1 };
    Int32 repeatRate    { 32 };
    Int32 repeatDelay   { 500 };
    bool modifiersChanged { true };
};

#endif // LKEYBOARDPRIVATE_H
