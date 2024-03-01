#include <LDNDManager.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LCompositor.h>
#include <LDataSource.h>
#include <LDNDIconRole.h>
#include <LPointerButtonEvent.h>
#include <LCursor.h>
#include <LTouch.h>
#include <LTouchDownEvent.h>
#include <LTouchPoint.h>

using namespace Louvre;

//! [startDragRequest]
void LDNDManager::startDragRequest()
{
    // Left pointer button click
    if (triggeringEvent().type() == LEvent::Type::Pointer && origin()->hasPointerFocus() && seat()->pointer()->isButtonPressed(LPointerButtonEvent::Left))
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
void LDNDManager::cancelled()
{
    if (icon())
        icon()->surface()->repaintOutputs();
}
//! [cancelled]
