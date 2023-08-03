#include "Keyboard.h"
#include "Global.h"
#include <LScene.h>

Keyboard::Keyboard(Params *params) : LKeyboard(params) {}

void Keyboard::keyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    G::scene()->handleKeyModifiersEvent(depressed, latched, locked, group);
}

void Keyboard::keyEvent(UInt32 keyCode, UInt32 keyState)
{
    G::scene()->handleKeyEvent(keyCode, keyState);
}
