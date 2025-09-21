#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerHoldBeginEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerHoldBeginEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerHoldBeginEvent(*this);
  }
}
