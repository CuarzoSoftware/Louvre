#ifndef LKEYBOARDPRIVATE_H
#define LKEYBOARDPRIVATE_H

#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/LKeyboard.h>
#include <CZ/Louvre/Events/LKeyboardModifiersEvent.h>
#include <vector>

using namespace Louvre;
using namespace Louvre::Protocols;

LPRIVATE_CLASS(LKeyboard)
    std::vector<UInt32> pressedKeys;
    CZWeak<LSurface> focus;
    LKeyboardModifiersEvent::Modifiers currentModifiersState { 0 };
    LKeyboardModifiersEvent::Modifiers prevModifiersState { 0 };
    CZWeak<LSurface> grab;
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
