#ifndef CLOCK_H
#define CLOCK_H

#include <LRect.h>
#include <LTimer.h>

#include <memory>

#include "../../common/TextRenderer.h"

using namespace Louvre;

class Clock {
 public:
  Clock() noexcept;
  void updateClockText();
  void updateClockTexture();
  static Int32 millisecondsUntilNextMinute();

  std::unique_ptr<LTexture> texture;
  std::unique_ptr<TextRenderer> font;
  LTimer minuteTimer;
  char text[128];
};

#endif  // CLOCK_H
