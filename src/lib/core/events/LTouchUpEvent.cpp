#include <LCompositor.h>
#include <LSeat.h>
#include <LTouch.h>
#include <LTouchUpEvent.h>

using namespace Louvre;

void LTouchUpEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;

    seat()->onEvent(*this);
    seat()->touch()->touchUpEvent(*this);
  }
}
