#include "Touch.h"

#include <LCursor.h>
#include <LScene.h>
#include <LTouchDownEvent.h>
#include <LTouchMoveEvent.h>

#include "Global.h"

void Touch::touchDownEvent(const LTouchDownEvent &event) {
  if (!cursor()->output()) return;

  const LPointF globalPos{toGlobal(cursor()->output(), event.pos())};
  G::scene()->handleTouchDownEvent(event, globalPos);
}

void Touch::touchMoveEvent(const LTouchMoveEvent &event) {
  if (!cursor()->output()) return;

  const LPointF globalPos{toGlobal(cursor()->output(), event.pos())};
  G::scene()->handleTouchMoveEvent(event, globalPos);
}

void Touch::touchUpEvent(const LTouchUpEvent &event) {
  G::scene()->handleTouchUpEvent(event);
}

void Touch::touchFrameEvent(const LTouchFrameEvent &event) {
  G::scene()->handleTouchFrameEvent(event);
}

void Touch::touchCancelEvent(const LTouchCancelEvent &event) {
  G::scene()->handleTouchCancelEvent(event);
}
