#include <LPointerSwipeEndEvent.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LPointer.h>

using namespace Louvre;

void LPointerSwipeEndEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        if (!seat()->eventFilter(*this))
            return;
        seat()->onEvent(*this);
        seat()->pointer()->pointerSwipeEndEvent(*this);
    }
}
