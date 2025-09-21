#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerPinchEndEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerPinchEndEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerPinchEndEvent(*this);
  }
}
