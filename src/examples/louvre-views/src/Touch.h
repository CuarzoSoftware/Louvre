#ifndef TOUCH_H
#define TOUCH_H

#include <LTouch.h>

using namespace Louvre;

class Touch final : public LTouch {
 public:
  using LTouch::LTouch;

  void touchDownEvent(const LTouchDownEvent &event) override;
  void touchMoveEvent(const LTouchMoveEvent &event) override;
  void touchUpEvent(const LTouchUpEvent &event) override;
  void touchFrameEvent(const LTouchFrameEvent &event) override;
  void touchCancelEvent(const LTouchCancelEvent &event) override;
};

#endif  // TOUCH_H
