#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Core/Events/CZTouchDownEvent.h>
#include <CZ/Louvre/Seat/LTouchPoint.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>

using namespace CZ;

//! [startDragRequest]
void LDND::startDragRequest()
{
    if (sessionLockManager()->state() != LSessionLockManager::Unlocked && sessionLockManager()->client() != origin()->client())
    {
        cancel();
        return;
    }

    // Left pointer button click
    if (!triggeringEvent().isTouchEvent() && origin()->hasPointerFocus() && seat()->pointer()->isButtonPressed(BTN_LEFT))
    {
        seat()->pointer()->setDraggingSurface(nullptr);

        if (icon())
            icon()->surface()->setPos(cursor()->pos());

        return;
    }
    // Touch down event
    else if (triggeringEvent().type() == CZEvent::Type::TouchDown)
    {
        const auto &touchDownEvent { static_cast<const CZTouchDownEvent&>(triggeringEvent()) };

        const LTouchPoint *tp { seat()->touch()->findTouchPoint(touchDownEvent.id) };

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
    if (!triggeringEvent().isTouchEvent())
    {
        cursor()->setVisible(true);
        cursor()->setSource({});
    }
}
//! [dropped]
