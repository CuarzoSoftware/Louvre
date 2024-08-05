#include <LPointerSwipeUpdateEvent.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LPointer.h>

using namespace Louvre;

void LPointerSwipeUpdateEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        seat()->onEvent(*this);
        seat()->pointer()->pointerSwipeUpdateEvent(*this);
    }
}
