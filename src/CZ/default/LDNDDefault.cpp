#include <CZ/Louvre/LDND.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LPointer.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/Events/LPointerButtonEvent.h>
#include <CZ/Louvre/LCursor.h>
#include <CZ/Louvre/LTouch.h>
#include <CZ/Louvre/Events/LTouchDownEvent.h>
#include <CZ/Louvre/LTouchPoint.h>
#include <CZ/Louvre/LSessionLockManager.h>

using namespace Louvre;

//! [startDragRequest]
void LDND::startDragRequest()
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
void LDND::cancelled()
{
    if (icon())
        icon()->surface()->repaintOutputs();
}

//! [cancelled]

//! [dropped]
void LDND::dropped()
{
    if (triggeringEvent().type() != LEvent::Type::Touch)
        cursor()->useDefault();
}
//! [dropped]
