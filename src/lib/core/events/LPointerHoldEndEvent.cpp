#include <LPointerHoldEndEvent.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LPointer.h>

using namespace Louvre;

void LPointerHoldEndEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        if (!seat()->eventFilter(*this))
            return;
        seat()->onEvent(*this);
        seat()->pointer()->pointerHoldEndEvent(*this);
    }
}
