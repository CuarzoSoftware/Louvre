#include <CZ/Louvre/Events/LPointerSwipeBeginEvent.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LPointer.h>

using namespace Louvre;

void LPointerSwipeBeginEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        if (!seat()->eventFilter(*this))
            return;
        seat()->onEvent(*this);
        seat()->pointer()->pointerSwipeBeginEvent(*this);
    }
}
