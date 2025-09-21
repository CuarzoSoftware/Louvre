#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerSwipeUpdateEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerSwipeUpdateEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerSwipeUpdateEvent(*this);
  }
}
