#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Core/Events/CZTouchDownEvent.h>
#include <CZ/Core/Events/CZTouchMoveEvent.h>
#include <CZ/Core/Events/CZTouchUpEvent.h>
#include <CZ/Core/Events/CZTouchFrameEvent.h>
#include <CZ/Core/Events/CZTouchCancelEvent.h>
#include <CZ/Louvre/Roles/LToplevelResizeSession.h>
#include <CZ/Louvre/Roles/LToplevelMoveSession.h>
#include <CZ/Louvre/Seat/LTouchPoint.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Seat/LSeat.h>

using namespace CZ;

//! [touchDownEvent]
void LTouch::touchDownEvent(const CZTouchDownEvent &event)
{
    if (!cursor()->output()) return;

    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };

    LTouchPoint *tp { createOrGetTouchPoint(event) };
    const SkPoint globalPos { toGlobal(cursor()->output(), event.pos) };
    LSurface *surface { seat()->surfaceAt(globalPos.x(), globalPos.y()) };

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
void LTouch::touchMoveEvent(const CZTouchMoveEvent &event)
{
    if (!cursor()->output()) return;

    LTouchPoint *tp { findTouchPoint(event.id) };

    if (!tp)
        return;

    const SkPoint globalPos { toGlobal(cursor()->output(), event.pos) };

    // Handle DND session
    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().isTouchEvent())
    {
        const Int32 id { GetTouchEventId(dnd.triggeringEvent()) };

        if (id == tp->id())
        {
            if (dnd.icon())
            {
                dnd.icon()->surface()->setPos(globalPos);
                dnd.icon()->surface()->repaintOutputs();
            }

            LSurface *surface { seat()->surfaceAt(globalPos.x(), globalPos.y()) };

            if (surface)
            {
                if (dnd.focus() == surface)
                    dnd.sendMoveEvent(globalPos - SkPoint(surface->rolePos().x(), surface->rolePos().y()), event.ms);
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
        if (!session->triggeringEvent().isTouchEvent())
            continue;

        const Int32 id { GetTouchEventId(session->triggeringEvent()) };

        if (id == tp->id())
        {
            activeResizing = true;
            session->updateDragPoint(SkIPoint(globalPos.x(), globalPos.y()));
            session->toplevel()->surface()->repaintOutputs();

            if (session->toplevel()->isMaximized())
                session->toplevel()->configureState(session->toplevel()->pendingConfiguration().windowState &~ CZWinMaximized);
        }
    }

    if (activeResizing)
        return;

    bool activeMoving { false };

    for (LToplevelMoveSession *session : seat()->toplevelMoveSessions())
    {
        if (!session->triggeringEvent().isTouchEvent())
            continue;

        const Int32 id { GetTouchEventId(session->triggeringEvent()) };

        if (id == tp->id())
        {
            activeMoving = true;
            session->updateDragPoint(SkIPoint(globalPos.x(), globalPos.y()));
            session->toplevel()->surface()->repaintOutputs();

            if (session->toplevel()->isMaximized())
                session->toplevel()->configureState(session->toplevel()->pendingConfiguration().windowState &~ CZWinMaximized);
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
void LTouch::touchUpEvent(const CZTouchUpEvent &event)
{
    LTouchPoint *tp { findTouchPoint(event.id) };

    if (!tp)
        return;

    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().isTouchEvent())
    {        
        const Int32 id { GetTouchEventId(dnd.triggeringEvent()) };

        if (id == tp->id())
            dnd.drop();
    }

    // Stop touch toplevel resizing sessions
    for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
    {
        if ((*it)->triggeringEvent().isTouchEvent())
        {
            const Int32 id { GetTouchEventId((*it)->triggeringEvent()) };

            if (id == tp->id())
            {
                it = (*it)->stop();
                continue;
            }
        }

        it++;
    }

    for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
    {
        if ((*it)->triggeringEvent().isTouchEvent())
        {
            const Int32 id { GetTouchEventId((*it)->triggeringEvent()) };

            if (id == tp->id())
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
void LTouch::touchFrameEvent(const CZTouchFrameEvent &event)
{
    // Released touch points are destroyed after sending this event
    sendFrameEvent(event);
}
//! [touchFrameEvent]

//! [touchCancelEvent]
void LTouch::touchCancelEvent(const CZTouchCancelEvent &event)
{
    LDND &dnd { *seat()->dnd() };

    if (dnd.dragging() && dnd.triggeringEvent().isTouchEvent())
        dnd.drop();

    // Stop touch toplevel resizing sessions
    for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
    {
        if ((*it)->triggeringEvent().isTouchEvent())
            it = (*it)->stop();
        else
            it++;
    }

    // Stop touch toplevel moving sessions
    for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
    {
        if ((*it)->triggeringEvent().isTouchEvent())
            it = (*it)->stop();
        else
            it++;
    }

    // All touch points are destroyed
    sendCancelEvent(event);
}
//! [touchCancelEvent]

bool LTouch::event(const CZEvent &e) noexcept
{
    switch (e.type())
    {
    case CZEvent::Type::TouchMove:
        touchMoveEvent((const CZTouchMoveEvent&)e);
        break;
    case CZEvent::Type::TouchDown:
        touchDownEvent((const CZTouchDownEvent&)e);
        break;
    case CZEvent::Type::TouchUp:
        touchUpEvent((const CZTouchUpEvent&)e);
        break;
    case CZEvent::Type::TouchFrame:
        touchFrameEvent((const CZTouchFrameEvent&)e);
        break;
    case CZEvent::Type::TouchCancel:
        touchCancelEvent((const CZTouchCancelEvent&)e);
        break;
    default:
        break;
    }

    return LFactoryObject::event(e);
}
