#include <LPointerScrollEvent.h>
#include <LCompositor.h>
#include <LPointer.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerScrollEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
        seat()->pointer()->pointerScrollEvent(*this);
}
