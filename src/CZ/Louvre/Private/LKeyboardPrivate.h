#ifndef LKEYBOARDPRIVATE_H
#define LKEYBOARDPRIVATE_H

#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Core/Events/CZKeyboardModifiersEvent.h>

using namespace CZ;
using namespace CZ::Protocols;

LPRIVATE_CLASS(LKeyboard)
    CZWeak<LSurface> focus;
    CZWeak<LSurface> grab;
    std::shared_ptr<CZKeymap> keymap;
    Int32 repeatRate    { 32 };
    Int32 repeatDelay   { 500 };
};

#endif // LKEYBOARDPRIVATE_H
