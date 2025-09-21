#include <LCompositor.h>
#include <LCursor.h>
#include <LDND.h>
#include <LDNDIconRole.h>
#include <LPointer.h>
#include <LPointerButtonEvent.h>
#include <LSeat.h>
#include <LSessionLockManager.h>
#include <LTouch.h>
#include <LTouchDownEvent.h>
#include <LTouchPoint.h>

using namespace Louvre;

//! [startDragRequest]
void LDND::startDragRequest() {
  if (sessionLockManager()->state() != LSessionLockManager::Unlocked &&
      sessionLockManager()->client() != origin()->client()) {
    cancel();
    return;
  }

  // Left pointer button click
  if (triggeringEvent().type() != LEvent::Type::Touch &&
      origin()->hasPointerFocus() &&
      seat()->pointer()->isButtonPressed(LPointerButtonEvent::Left)) {
    seat()->pointer()->setDraggingSurface(nullptr);

    if (icon()) icon()->surface()->setPos(cursor()->pos());

    return;
  }
  // Touch down event
  else if (triggeringEvent().type() == LEvent::Type::Touch &&
           triggeringEvent().subtype() == LEvent::Subtype::Down) {
    const LTouchDownEvent &touchDownEvent{
        (const LTouchDownEvent &)triggeringEvent()};

    const LTouchPoint *tp{seat()->touch()->findTouchPoint(touchDownEvent.id())};

    if (tp && tp->surface() == origin()) {
      if (icon())
        icon()->surface()->setPos(
            LTouch::toGlobal(cursor()->output(), tp->pos()));
      return;
    }
  }

  cancel();
}
//! [startDragRequest]

//! [cancelled]
void LDND::cancelled() {
  if (icon()) icon()->surface()->repaintOutputs();
}

//! [cancelled]

//! [dropped]
void LDND::dropped() {
  if (triggeringEvent().type() != LEvent::Type::Touch) cursor()->useDefault();
}
//! [dropped]
