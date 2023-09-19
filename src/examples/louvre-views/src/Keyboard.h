#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <LKeyboard.h>

using namespace Louvre;

class Keyboard : public LKeyboard
{
public:
    Keyboard(Params *params);

    void keyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group) override;
    void keyEvent(UInt32 keyCode, KeyState keyState) override;
    void focusChanged() override;
};

#endif // KEYBOARD_H
