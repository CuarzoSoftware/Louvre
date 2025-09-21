#include <LCompositor.h>
#include <LPointer.h>
#include <LPointerScrollEvent.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerScrollEvent::notify() {
  if (compositor()->state() == LCompositor::Initialized) {
    if (!seat()->eventFilter(*this)) return;
    seat()->onEvent(*this);
    seat()->pointer()->pointerScrollEvent(*this);
  }
}
