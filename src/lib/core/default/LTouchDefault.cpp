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
#include <LToplevelResizeSession.h>
#include <LToplevelMoveSession.h>
#include <LTouchPoint.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LLog.h>

void LTouch::touchDownEvent(const LTouchDownEvent &event)
{
    LTouchPoint *tp { createOrGetTouchPoint(event) };
    const LPointF globalPos { toGlobal(cursor()->output(), event.pos()) };
    LSurface *surface { surfaceAt(globalPos) };

    if (surface)
    {
        event.localPos = globalPos - surface->rolePos();
        seat()->keyboard()->setFocus(surface);
        tp->sendDownEvent(event, surface);
        surface->raise();
    }
    else
        tp->sendDownEvent(event);
}

void LTouch::touchMoveEvent(const LTouchMoveEvent &event)
{
    LTouchPoint *tp { findTouchPoint(event.id()) };

    if (!tp)
        return;

    const LPointF globalPos { toGlobal(cursor()->output(), event.pos()) };

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

    bool activeResizing { false };

    for (LToplevelResizeSession *session : seat()->toplevelResizeSessions())
    {
        if (session->triggeringEvent().type() == LEvent::Type::Touch && session->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            LTouchDownEvent &touchDownEvent = (LTouchDownEvent&)session->triggeringEvent();

            if (touchDownEvent.id() == tp->id())
            {
                activeResizing = true;
                session->updateDragPoint(globalPos);
                session->toplevel()->surface()->repaintOutputs();

                if (session->toplevel()->maximized())
                    session->toplevel()->configure(session->toplevel()->pendingStates() &~ LToplevelRole::Maximized);
            }
        }
    }

    if (activeResizing)
        return;

    bool activeMoving { false };

    for (auto session : seat()->toplevelMoveSessions())
    {
        if (session->triggeringEvent().type() == LEvent::Type::Touch && session->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            auto touchDownEvent { (const LTouchDownEvent&)session->triggeringEvent() };

            if (touchDownEvent.id() == tp->id())
            {
                activeMoving = true;
                session->updateDragPoint(globalPos);
                session->toplevel()->surface()->repaintOutputs();

                if (session->toplevel()->maximized())
                    session->toplevel()->configure(session->toplevel()->pendingStates() &~ LToplevelRole::Maximized);
            }
        }
    }

    if (activeMoving)
        return;

    if (tp->surface())
    {
        event.localPos = globalPos - tp->surface()->rolePos();
        tp->sendMoveEvent(event);
    }
    else
        tp->sendMoveEvent(event);
}

void LTouch::touchUpEvent(const LTouchUpEvent &event)
{
    LTouchPoint *tp { findTouchPoint(event.id()) };

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
    */

    // Stop touch toplevel resizing sessions
    for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch && (*it)->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            LTouchDownEvent &downEvent = (LTouchDownEvent&)(*it)->triggeringEvent();

            if (downEvent.id() == tp->id())
            {
                it = (*it)->stop();
                continue;
            }
        }

        it++;
    }

    for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch && (*it)->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            auto downEvent = (const LTouchDownEvent&)(*it)->triggeringEvent();

            if (downEvent.id() == tp->id())
            {
                it = (*it)->stop();
                continue;
            }
        }

        it++;
    }

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
    // Stop touch toplevel resizing sessions
    for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch)
            it = (*it)->stop();
        else
            it++;
    }

    // Stop touch toplevel moving sessions
    for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch)
            it = (*it)->stop();
        else
            it++;
    }

    // All touch points are destroyed
    sendCancelEvent(event);
}
