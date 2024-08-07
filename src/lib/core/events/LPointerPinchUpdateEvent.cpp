#include <LPointerPinchUpdateEvent.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LPointer.h>

using namespace Louvre;

void LPointerPinchUpdateEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        seat()->onEvent(*this);
        seat()->pointer()->pointerPinchUpdateEvent(*this);
    }
}
