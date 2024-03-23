#include <LDND.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LCompositor.h>
#include <LDNDIconRole.h>
#include <LPointerButtonEvent.h>
#include <LCursor.h>
#include <LTouch.h>
#include <LTouchDownEvent.h>
#include <LTouchPoint.h>
#include <LSessionLockManager.h>

using namespace Louvre;

//! [startDragRequest]
void LDND::startDragRequest() noexcept
{
    if (sessionLockManager()->state() != LSessionLockManager::Unlocked && sessionLockManager()->client() != origin()->client())
    {
        cancel();
        return;
    }

    // Left pointer button click
    if (triggeringEvent().type() != LEvent::Type::Touch && origin()->hasPointerFocus() && seat()->pointer()->isButtonPressed(LPointerButtonEvent::Left))
    {
        seat()->pointer()->setDraggingSurface(nullptr);

        if (icon())
            icon()->surface()->setPos(cursor()->pos());

        return;
    }
    // Touch down event
    else if (triggeringEvent().type() == LEvent::Type::Touch && triggeringEvent().subtype() == LEvent::Subtype::Down)
    {
        const LTouchDownEvent &touchDownEvent { (const LTouchDownEvent&)triggeringEvent() };

        const LTouchPoint *tp { seat()->touch()->findTouchPoint(touchDownEvent.id()) };

        if (tp && tp->surface() == origin())
        {
            if (icon())
                icon()->surface()->setPos(LTouch::toGlobal(cursor()->output(), tp->pos()));
            return;
        }
    }

    cancel();
}
//! [startDragRequest]

//! [cancelled]
void LDND::cancelled() noexcept
{
    if (icon())
        icon()->surface()->repaintOutputs();
}
//! [cancelled]
