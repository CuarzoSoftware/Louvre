#include <CZ/Louvre/Events/LPointerHoldBeginEvent.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LPointer.h>

using namespace Louvre;

void LPointerHoldBeginEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        if (!seat()->eventFilter(*this))
            return;
        seat()->onEvent(*this);
        seat()->pointer()->pointerHoldBeginEvent(*this);
    }
}
