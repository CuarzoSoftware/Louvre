#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerPinchUpdateEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerPinchUpdateEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerPinchUpdateEvent(*this);
  }
}
