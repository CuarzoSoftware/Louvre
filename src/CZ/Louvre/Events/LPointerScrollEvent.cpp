#include <CZ/Louvre/Events/LPointerScrollEvent.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LPointer.h>
#include <CZ/Louvre/LSeat.h>

using namespace Louvre;

void LPointerScrollEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        if (!seat()->eventFilter(*this))
            return;
        seat()->onEvent(*this);
        seat()->pointer()->pointerScrollEvent(*this);
    }
}
