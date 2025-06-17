#include <CZ/Louvre/Events/LTouchDownEvent.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LTouch.h>

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
