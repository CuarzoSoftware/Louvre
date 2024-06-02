#include <LSessionLockManager.h>
#include <LTouchDownEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>
#include <LTouchFrameEvent.h>
#include <LTouchCancelEvent.h>
#include <LToplevelResizeSession.h>
#include <LToplevelMoveSession.h>
#include <LTouchPoint.h>
#include <LTouch.h>
#include <LKeyboard.h>
#include <LDNDIconRole.h>
#include <LDND.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LSurface.h>
#include <LSeat.h>

using namespace Louvre;

//! [touchDownEvent]
void LTouch::touchDownEvent(const LTouchDownEvent &event)
{
    if (!cursor()->output()) return;

    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };

    LTouchPoint *tp { createOrGetTouchPoint(event) };
    const LPointF globalPos { toGlobal(cursor()->output(), event.pos()) };
    LSurface *surface { surfaceAt(globalPos) };

    if (surface)
    {
        if (sessionLocked && surface->client() != sessionLockManager()->client())
            return;

        event.localPos = globalPos - surface->rolePos();

        if (!seat()->keyboard()->focus() || !surface->isSubchildOf(seat()->keyboard()->focus()))
            seat()->keyboard()->setFocus(surface);

        tp->sendDownEvent(event, surface);
        surface->raise();
    }
    else
    {
        tp->sendDownEvent(event);
        seat()->dismissPopups();
    }
}
//! [touchDownEvent]

//! [touchMoveEvent]
void LTouch::touchMoveEvent(const LTouchMoveEvent &event)
{
    if (!cursor()->output()) return;

    LTouchPoint *tp { findTouchPoint(event.id()) };

    if (!tp)
        return;

    const LPointF globalPos { toGlobal(cursor()->output(), event.pos()) };

    // Handle DND session
    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().type() == LEvent::Type::Touch && dnd.triggeringEvent().subtype() == LEvent::Subtype::Down)
    {
        const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(dnd.triggeringEvent()) };

        if (touchDownEvent.id() == tp->id())
        {
            if (dnd.icon())
            {
                dnd.icon()->surface()->setPos(globalPos);
                dnd.icon()->surface()->repaintOutputs();
            }

            LSurface *surface { surfaceAt(globalPos) };

            if (surface)
            {
                if (dnd.focus() == surface)
                    dnd.sendMoveEvent(globalPos - surface->rolePos(), event.ms());
                else
                    dnd.setFocus(surface, globalPos - surface->rolePos());
            }
            else
                dnd.setFocus(nullptr, LPoint());
        }
    }

    bool activeResizing { false };

    for (LToplevelResizeSession *session : seat()->toplevelResizeSessions())
    {
        if (session->triggeringEvent().type() == LEvent::Type::Touch && session->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(session->triggeringEvent()) };

            if (touchDownEvent.id() == tp->id())
            {
                activeResizing = true;
                session->updateDragPoint(globalPos);
                session->toplevel()->surface()->repaintOutputs();

                if (session->toplevel()->maximized())
                    session->toplevel()->configureState(session->toplevel()->pendingConfiguration().state &~ LToplevelRole::Maximized);
            }
        }
    }

    if (activeResizing)
        return;

    bool activeMoving { false };

    for (LToplevelMoveSession *session : seat()->toplevelMoveSessions())
    {
        if (session->triggeringEvent().type() == LEvent::Type::Touch && session->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(session->triggeringEvent()) };

            if (touchDownEvent.id() == tp->id())
            {
                activeMoving = true;
                session->updateDragPoint(globalPos);
                session->toplevel()->surface()->repaintOutputs();

                if (session->toplevel()->maximized())
                    session->toplevel()->configureState(session->toplevel()->pendingConfiguration().state &~ LToplevelRole::Maximized);
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
//! [touchMoveEvent]

//! [touchUpEvent]
void LTouch::touchUpEvent(const LTouchUpEvent &event)
{
    LTouchPoint *tp { findTouchPoint(event.id()) };

    if (!tp)
        return;

    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().type() == LEvent::Type::Touch && dnd.triggeringEvent().subtype() == LEvent::Subtype::Down)
    {
        const LTouchDownEvent &touchDownEvent { static_cast<const LTouchDownEvent&>(dnd.triggeringEvent()) };

        if (touchDownEvent.id() == tp->id())
            dnd.drop();
    }

    // Stop touch toplevel resizing sessions
    for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch && (*it)->triggeringEvent().subtype() == LEvent::Subtype::Down)
        {
            const LTouchDownEvent &downEvent { static_cast<const LTouchDownEvent&>((*it)->triggeringEvent()) };

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
            const LTouchDownEvent &downEvent { static_cast<const LTouchDownEvent&>((*it)->triggeringEvent()) };

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
//! [touchUpEvent]

//! [touchFrameEvent]
void LTouch::touchFrameEvent(const LTouchFrameEvent &event)
{
    // Released touch points are destroyed after sending this event
    sendFrameEvent(event);
}
//! [touchFrameEvent]

//! [touchCancelEvent]
void LTouch::touchCancelEvent(const LTouchCancelEvent &event)
{
    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().type() == LEvent::Type::Touch)
        dnd.drop();

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
//! [touchCancelEvent]
