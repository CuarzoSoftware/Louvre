#include <CZ/Louvre/Private/LPointerPrivate.h>
#include <CZ/Louvre/Events/LPointerButtonEvent.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LUtils.h>
#include <CZ/Louvre/LSeat.h>

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
