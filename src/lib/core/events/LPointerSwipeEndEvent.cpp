#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerSwipeEndEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerSwipeEndEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerSwipeEndEvent(*this);
  }
}
