#include <LCompositor.h>
#include <LSeat.h>
#include <LTouch.h>
#include <LTouchFrameEvent.h>

using namespace Louvre;

void LTouchFrameEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;

    seat()->onEvent(*this);
    seat()->touch()->touchFrameEvent(*this);
  }
}
