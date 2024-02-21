#include <LTouchFrameEvent.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LTouch.h>

using namespace Louvre;

void LTouchFrameEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
        seat()->touch()->touchFrameEvent(*this);
}
