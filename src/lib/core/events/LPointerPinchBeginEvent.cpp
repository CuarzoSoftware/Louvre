#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerPinchBeginEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerPinchBeginEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerPinchBeginEvent(*this);
  }
}
