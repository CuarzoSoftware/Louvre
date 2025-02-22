#include <private/LPointerPrivate.h>
#include <LPointerButtonEvent.h>
#include <LCompositor.h>
#include <LUtils.h>
#include <LSeat.h>

using namespace Louvre;

void LPointerButtonEvent::notify()
{
    if (compositor()->state() == LCompositor::Initialized)
    {
        if (!seat()->eventFilter(*this))
            return;

        seat()->onEvent(*this);

        if (state() == Pressed)
            seat()->pointer()->imp()->pressedButtons.push_back(button());
        else
            LVectorRemoveOneUnordered(seat()->pointer()->imp()->pressedButtons, button());

        seat()->pointer()->pointerButtonEvent(*this);
    }
}
