#include <CZ/Louvre/Events/LPointerSwipeEndEvent.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LPointer.h>

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
