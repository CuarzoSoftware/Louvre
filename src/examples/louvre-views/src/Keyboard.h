#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <LKeyboard.h>

using namespace Louvre;

class Keyboard final : public LKeyboard {
 public:
  Keyboard(const void *params);

  void keyEvent(const LKeyboardKeyEvent &event) override;
  void focusChanged() override;
};

#endif  // KEYBOARD_H
