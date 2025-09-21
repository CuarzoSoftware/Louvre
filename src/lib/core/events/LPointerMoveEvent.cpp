#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerMoveEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerMoveEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerMoveEvent(*this);
  }
}
