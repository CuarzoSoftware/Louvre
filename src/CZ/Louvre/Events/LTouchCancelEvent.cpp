#include <CZ/Louvre/Events/LTouchCancelEvent.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LTouch.h>

using namespace Louvre;

void LTouchCancelEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        if (!seat()->eventFilter(*this))
            return;

        seat()->onEvent(*this);
        seat()->touch()->touchCancelEvent(*this);
    }
}
