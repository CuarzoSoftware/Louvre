#include <LCompositor.h>
#include <LSeat.h>
#include <LTouch.h>
#include <LTouchCancelEvent.h>

using namespace Louvre;

void LTouchCancelEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;

    seat()->onEvent(*this);
    seat()->touch()->touchCancelEvent(*this);
  }
}
