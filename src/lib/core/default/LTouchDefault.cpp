#include <private/LTouchPrivate.h>
#include <LKeyboard.h>
#include <LDNDManager.h>
#include <LDNDIconRole.h>
#include <LInputDevice.h>
#include <LTouchDownEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>
#include <LTouchFrameEvent.h>
#include <LTouchCancelEvent.h>
// TODO #include <LToplevelResizeSession.h>
// TODO #include <LToplevelMoveSession.h>
#include <LTouchPoint.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LLog.h>

void LTouch::touchDownEvent(const LTouchDownEvent &event)
{
    // Creates or returns an existing touch point with the event id
    LTouchPoint *tp = createTouchPoint(event);

    // Transform touch position to global position
    LPointF globalPos = toGlobal(cursor()->output(), event.pos());

    // Check if a surface was touched
    LSurface *surface = surfaceAt(globalPos);

    if (surface)
    {
        event.localPos = globalPos - surface->rolePos();
        tp->sendDownEvent(event, surface);
        seat()->keyboard()->setFocus(surface);
        surface->raise();
    }
    else
        tp->sendDownEvent(event);
}

void LTouch::touchMoveEvent(const LTouchMoveEvent &event)
{
    LTouchPoint *tp = findTouchPoint(event.id());

    if (!tp)
        return;

    // Transform touch position to global position
    LPointF globalPos = toGlobal(cursor()->output(), event.pos());

    /* TODO
    // Handle DND session
    LDNDManager *dnd = seat()->dndManager();

    if (dnd->dragging() && dnd->triggererEvent().type() == LEvent::Type::Touch)
    {
        LTouchDownEvent &touchDownEvent = (LTouchDownEvent&)dnd->triggererEvent();

        if (touchDownEvent.id() == tp->id())
        {
            if (dnd->icon())
            {
                dnd->icon()->surface()->setPos(globalPos);
                dnd->icon()->surface()->repaintOutputs();
            }

            LSurface *surface = surfaceAt(globalPos);

            if (surface)
            {
                if (dnd->focus() == surface)
                    dnd->sendMoveEvent(globalPos - surface->rolePos(), event.ms());
                else
                    dnd->setFocus(surface, globalPos - surface->rolePos());
            }
            else
                dnd->setFocus(nullptr, LPoint());
        }
    }*/

    /* TODO
    bool activeResizing = false;

    for (LToplevelResizeSession *session : seat()->resizeSessions())
    {
        if (session->triggeringEvent().type() == LEvent::Type::Touch && session->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            LTouchDownEvent &touchDownEvent = (LTouchDownEvent&)session->triggeringEvent();

            if (touchDownEvent.id() == tp->id())
            {
                activeResizing = true;
                session->setResizePointPos(globalPos);
                session->toplevel()->surface()->repaintOutputs();

                if (session->toplevel()->maximized())
                    session->toplevel()->configure(session->toplevel()->pendingState() &~ LToplevelRole::Maximized);
            }
        }
    } */

    /* TODO
    if (activeResizing)
        return;

    bool activeMoving = false;

    for (LToplevelMoveSession *session : seat()->moveSessions())
    {
        if (session->triggeringEvent().type() == LEvent::Type::Touch && session->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            LTouchDownEvent &touchDownEvent = (LTouchDownEvent&)session->triggeringEvent();

            if (touchDownEvent.id() == tp->id())
            {
                activeMoving = true;
                session->setMovePointPos(globalPos);
                session->toplevel()->surface()->repaintOutputs();

                if (session->toplevel()->maximized())
                    session->toplevel()->configure(session->toplevel()->pendingState() &~ LToplevelRole::Maximized);
            }
        }
    }

    if (activeMoving)
        return;
    */

    // Send the event
    if (tp->surface())
    {
        event.localPos = globalPos -tp->surface()->rolePos();
        tp->sendMoveEvent(event);
    }
    else
        tp->sendMoveEvent(event);
}

void LTouch::touchUpEvent(const LTouchUpEvent &event)
{
    LTouchPoint *tp = findTouchPoint(event.id());

    if (!tp)
        return;

    /* TODO
    LDNDManager *dnd = seat()->dndManager();

    if (dnd->dragging() && dnd->triggererEvent().type() == LEvent::Type::Touch)
    {
        LTouchDownEvent &touchDownEvent = (LTouchDownEvent&)dnd->triggererEvent();

        if (touchDownEvent.id() == tp->id())
            dnd->drop();
    }

    // Stop touch toplevel resizing sessions
    for (std::list<LToplevelResizeSession*>::const_iterator it = seat()->resizeSessions().begin(); it != seat()->resizeSessions().end(); it++)
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch && (*it)->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            LTouchDownEvent &downEvent = (LTouchDownEvent&)(*it)->triggeringEvent();

            if (downEvent.id() == tp->id())
                it = (*it)->stop();
        }

    // Stop touch toplevel moving sessions
    for (std::list<LToplevelMoveSession*>::const_iterator it = seat()->moveSessions().begin(); it != seat()->moveSessions().end(); it++)
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch && (*it)->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            LTouchDownEvent &downEvent = (LTouchDownEvent&)(*it)->triggeringEvent();

            if (downEvent.id() == tp->id())
                it = (*it)->stop();
        }
    */

    // Send the event
    tp->sendUpEvent(event);
}

void LTouch::touchFrameEvent(const LTouchFrameEvent &event)
{
    // Released touch points are destroyed after this event
    sendFrameEvent(event);
}

void LTouch::touchCancelEvent(const LTouchCancelEvent &event)
{
    /* TODO
    // Stop touch toplevel resizing sessions
    for (std::list<LToplevelResizeSession*>::const_iterator it = seat()->resizeSessions().begin(); it != seat()->resizeSessions().end(); it++)
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch)
                it = (*it)->stop();

    // Stop touch toplevel moving sessions
    for (std::list<LToplevelMoveSession*>::const_iterator it = seat()->moveSessions().begin(); it != seat()->moveSessions().end(); it++)
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch)
                it = (*it)->stop();
    */
    // All touch points are destroyed
    sendCancelEvent(event);
}
