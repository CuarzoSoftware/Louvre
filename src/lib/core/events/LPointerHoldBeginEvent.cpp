#include <LPointerHoldBeginEvent.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LPointer.h>

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
