#include <LTouchDownEvent.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LTouch.h>

using namespace Louvre;

void LTouchDownEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        if (!seat()->eventFilter(*this))
            return;

        seat()->onEvent(*this);
        seat()->touch()->touchDownEvent(*this);
    }
}
