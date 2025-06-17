#include <CZ/Louvre/Events/LTouchMoveEvent.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LTouch.h>

using namespace Louvre;

void LTouchMoveEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        if (!seat()->eventFilter(*this))
            return;

        seat()->onEvent(*this);
        seat()->touch()->touchMoveEvent(*this);
    }
}
