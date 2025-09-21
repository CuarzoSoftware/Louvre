#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerHoldEndEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerHoldEndEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerHoldEndEvent(*this);
  }
}
