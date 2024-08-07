#include <LTouchCancelEvent.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LTouch.h>

using namespace Louvre;

void LTouchCancelEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        seat()->onEvent(*this);
        seat()->touch()->touchCancelEvent(*this);
    }
}
