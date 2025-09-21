#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerSwipeBeginEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerSwipeBeginEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerSwipeBeginEvent(*this);
  }
}
