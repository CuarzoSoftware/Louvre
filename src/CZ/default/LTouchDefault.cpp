#include <CZ/Louvre/LSessionLockManager.h>
#include <CZ/Louvre/Events/LTouchDownEvent.h>
#include <CZ/Louvre/Events/LTouchMoveEvent.h>
#include <CZ/Louvre/Events/LTouchUpEvent.h>
#include <CZ/Louvre/Events/LTouchFrameEvent.h>
#include <CZ/Louvre/Events/LTouchCancelEvent.h>
#include <CZ/Louvre/Roles/LToplevelResizeSession.h>
#include <CZ/Louvre/Roles/LToplevelMoveSession.h>
#include <CZ/Louvre/LTouchPoint.h>
#include <CZ/Louvre/LTouch.h>
#include <CZ/Louvre/LKeyboard.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/LDND.h>
#include <CZ/Louvre/LCursor.h>
#include <CZ/Louvre/LOutput.h>
#include <CZ/Louvre/LSurface.h>
#include <CZ/Louvre/LSeat.h>

using namespace Louvre;

//! [touchDownEvent]
void LTouch::touchDownEvent(const LTouchDownEvent &event)
{
    if (!cursor()->output()) return;

    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };

    LTouchPoint *tp { createOrGetTouchPoint(event) };
    const SkPoint globalPos { toGlobal(cursor()->output(), event.pos()) };
    LSurface *surface { surfaceAt(globalPos.x(), globalPos.y()) };

    if (surface)
    {
        if (sessionLocked && surface->client() != sessionLockManager()->client())
            return;

        event.localPos.fX = globalPos.x() - surface->rolePos().x();
        event.localPos.fY = globalPos.y() - surface->rolePos().y();

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

    const SkPoint globalPos { toGlobal(cursor()->output(), event.pos()) };

    // Handle DND session
    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().type() == LEvent::Type::Touch)
    {
        const auto &touchEvent { static_cast<const LTouchEvent&>(dnd.triggeringEvent()) };

        if (touchEvent.id() == tp->id())
        {
            if (dnd.icon())
            {
                dnd.icon()->surface()->setPos(globalPos);
                dnd.icon()->surface()->repaintOutputs();
            }

            LSurface *surface { surfaceAt(globalPos.x(), globalPos.y()) };

            if (surface)
            {
                if (dnd.focus() == surface)
                    dnd.sendMoveEvent(globalPos - SkPoint(surface->rolePos().x(), surface->rolePos().y()), event.ms());
                else
                    dnd.setFocus(surface, globalPos - SkPoint(surface->rolePos().x(), surface->rolePos().y()));
            }
            else
                dnd.setFocus(nullptr, SkPoint());
        }
    }

    bool activeResizing { false };

    for (LToplevelResizeSession *session : seat()->toplevelResizeSessions())
    {
        if (session->triggeringEvent().type() == LEvent::Type::Touch)
        {
            const auto &touchEvent { static_cast<const LTouchEvent&>(session->triggeringEvent()) };

            if (touchEvent.id() == tp->id())
            {
                activeResizing = true;
                session->updateDragPoint(SkIPoint(globalPos.x(), globalPos.y()));
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
        if (session->triggeringEvent().type() == LEvent::Type::Touch)
        {
            const auto &touchEvent { static_cast<const LTouchEvent&>(session->triggeringEvent()) };

            if (touchEvent.id() == tp->id())
            {
                activeMoving = true;
                session->updateDragPoint(SkIPoint(globalPos.x(), globalPos.y()));
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
        event.localPos = globalPos - SkPoint(tp->surface()->rolePos().x(), tp->surface()->rolePos().y());
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

    if (dnd.dragging() && dnd.triggeringEvent().type() == LEvent::Type::Touch)
    {
        const auto &touchEvent { static_cast<const LTouchEvent&>(dnd.triggeringEvent()) };

        if (touchEvent.id() == tp->id())
            dnd.drop();
    }

    // Stop touch toplevel resizing sessions
    for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch)
        {
            const auto &touchEvent { static_cast<const LTouchEvent&>((*it)->triggeringEvent()) };

            if (touchEvent.id() == tp->id())
            {
                it = (*it)->stop();
                continue;
            }
        }

        it++;
    }

    for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
    {
        if ((*it)->triggeringEvent().type() == LEvent::Type::Touch)
        {
            const auto &touchEvent { static_cast<const LTouchEvent&>((*it)->triggeringEvent()) };

            if (touchEvent.id() == tp->id())
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
