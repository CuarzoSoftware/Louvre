#include <CZ/Utils/CZRegionUtils.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/LPointer.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LCursor.h>
#include <CZ/Louvre/LOutput.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Roles/LPopupRole.h>
#include <CZ/Louvre/LTime.h>
#include <CZ/Louvre/LKeyboard.h>
#include <CZ/Louvre/LDND.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/Events/LPointerMoveEvent.h>
#include <CZ/Louvre/Events/LPointerButtonEvent.h>
#include <CZ/Louvre/Roles/LToplevelMoveSession.h>
#include <CZ/Louvre/Roles/LToplevelResizeSession.h>
#include <CZ/Louvre/LClientCursor.h>
#include <CZ/Louvre/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>

using namespace Louvre;

//! [pointerMoveEvent]
void LPointer::pointerMoveEvent(const LPointerMoveEvent &event)
{
    // Update the cursor position
    cursor()->move(event.delta().x(), event.delta().y());

    bool pointerConstrained { false };

    if (focus())
    {
        SkPoint fpos { SkPoint(focus()->rolePos().x(), focus()->rolePos().y()) };

        // Attempt to enable the pointer constraint mode if the cursor is within the constrained region.
        if (focus()->pointerConstraintMode() != LSurface::PointerConstraintMode::Free)
        {
            if (focus()->pointerConstraintRegion().contains(cursor()->pos().x() - fpos.x(), cursor()->pos().y() - fpos.y()))
                focus()->enablePointerConstraint(true);
        }

        if (focus()->pointerConstraintEnabled())
        {
            if (focus()->pointerConstraintMode() == LSurface::PointerConstraintMode::Lock)
            {
                if (focus()->lockedPointerPosHint().x() >= 0.f)
                    cursor()->setPos(fpos + focus()->lockedPointerPosHint());
                else
                {
                    cursor()->move(-event.delta().x(), -event.delta().y());

                    const SkPoint closestPoint {
                        CZRegionUtils::ClosestPointFrom(focus()->pointerConstraintRegion(), cursor()->pos() - fpos)
                    };

                    cursor()->setPos(fpos + closestPoint);
                }
            }
            else /* Confined */
            {
                const SkPoint closestPoint {
                    CZRegionUtils::ClosestPointFrom(focus()->pointerConstraintRegion(), cursor()->pos() - fpos)
                };

                cursor()->setPos(fpos + closestPoint);
            }

            pointerConstrained = true;
        }
    }

    // Schedule repaint on outputs that intersect with the cursor where hardware composition is not supported.
    cursor()->repaintOutputs(true);

    const bool sessionLocked { compositor()->sessionLockManager()->state() != LSessionLockManager::Unlocked };
    const bool activeDND { seat()->dnd()->dragging() && seat()->dnd()->triggeringEvent().type() != LEvent::Type::Touch };

    if (activeDND)
    {
        if (seat()->dnd()->icon())
        {
            seat()->dnd()->icon()->surface()->setPos(cursor()->pos());
            seat()->dnd()->icon()->surface()->repaintOutputs();
            cursor()->setCursor(seat()->dnd()->icon()->surface()->client()->lastCursorRequest());
        }

        seat()->keyboard()->setFocus(nullptr);
        setDraggingSurface(nullptr);
        setFocus(nullptr);
    }

    bool activeResizing { false };

    for (LToplevelResizeSession *session : seat()->toplevelResizeSessions())
    {
        if (session->triggeringEvent().type() != LEvent::Type::Touch)
        {
            activeResizing = true;
            session->updateDragPoint(SkIPoint(cursor()->pos().x(), cursor()->pos().y()));
        }
    }

    if (activeResizing)
        return;

    bool activeMoving { false };

    for (LToplevelMoveSession *session : seat()->toplevelMoveSessions())
    {
        if (session->triggeringEvent().type() != LEvent::Type::Touch)
        {
            activeMoving = true;
            session->updateDragPoint(SkIPoint(cursor()->pos().x(), cursor()->pos().y()));
            session->toplevel()->surface()->repaintOutputs();

            if (session->toplevel()->maximized())
                session->toplevel()->configureState(session->toplevel()->pendingConfiguration().state &~ LToplevelRole::Maximized);
        }
    }

    if (activeMoving)
        return;

    // If a surface had the left pointer button held down
    if (draggingSurface())
    {
        event.localPos = cursor()->pos() - SkPoint(draggingSurface()->rolePos().x(), draggingSurface()->rolePos().y());
        sendMoveEvent(event);
        return;
    }

    // Find the first surface under the cursor
    LSurface *surface { pointerConstrained ? focus() : surfaceAt(cursor()->pos()) };

    if (surface)
    {
        if (sessionLocked && surface->client() != sessionLockManager()->client())
            return;

        event.localPos = cursor()->pos() - SkPoint(surface->rolePos().x(), surface->rolePos().y());

        if (activeDND)
        {
            if (seat()->dnd()->focus() == surface)
                seat()->dnd()->sendMoveEvent(event.localPos, event.ms());
            else
                seat()->dnd()->setFocus(surface, event.localPos);
        }
        else
        {
            if (focus() == surface)
                sendMoveEvent(event);
            else
                setFocus(surface, event.localPos);
        }

        cursor()->setCursor(surface->client()->lastCursorRequest());
    }
    else
    {
        if (activeDND)
            seat()->dnd()->setFocus(nullptr, SkPoint());
        else
        {
            setFocus(nullptr);
            cursor()->useDefault();
            cursor()->setVisible(true);
        }
    }
}
//! [pointerMoveEvent]

//! [pointerButtonEvent]
void LPointer::pointerButtonEvent(const LPointerButtonEvent &event)
{
    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };
    const bool activeDND { seat()->dnd()->dragging() && seat()->dnd()->triggeringEvent().type() != LEvent::Type::Touch };

    if (activeDND)
    {
        if (event.state() == LPointerButtonEvent::Released &&
            event.button() == LPointerButtonEvent::Left)
            seat()->dnd()->drop();

        seat()->keyboard()->setFocus(nullptr);
        setFocus(nullptr);
        setDraggingSurface(nullptr);
        return;
    }

    if (!focus())
    {
        LSurface *surface { surfaceAt(cursor()->pos()) };

        if (surface)
        {
            if (sessionLocked && surface->client() != sessionLockManager()->client())
                return;

            cursor()->setCursor(surface->client()->lastCursorRequest());
            seat()->keyboard()->setFocus(surface);
            setFocus(surface);
            sendButtonEvent(event);

            if (!surface->popup() && !surface->isPopupSubchild())
                seat()->dismissPopups();
        }
        else
        {
            seat()->keyboard()->setFocus(nullptr);
            seat()->dismissPopups();
        }

        return;
    }

    if (event.button() != LPointerButtonEvent::Left)
    {
        sendButtonEvent(event);
        return;
    }

    // Left button pressed
    if (event.state() == LPointerButtonEvent::Pressed)
    {
        // Keep a ref to continue sending it events after the cursor
        // leaves, if the left button remains pressed
        setDraggingSurface(focus());

        // Most apps close popups when they get keyboard focus,
        // probably because the parent loses it
        if (!focus()->popup() && !focus()->isPopupSubchild())
        {
            seat()->keyboard()->setFocus(focus());

            // Pointer focus may have changed within LKeyboard::focusChanged()
            if (!focus())
                return;
        }

        sendButtonEvent(event);

        if (focus()->toplevel() && !focus()->toplevel()->activated())
            focus()->toplevel()->configureState(focus()->toplevel()->pendingConfiguration().state | LToplevelRole::Activated);

        if (!focus()->popup() && !focus()->isPopupSubchild())
            seat()->dismissPopups();

        if (!focus() || focus() == compositor()->surfaces().back())
            return;

        if (focus()->parent())
            focus()->topmostParent()->raise();
        else
            focus()->raise();
    }
    // Left button released
    else
    {
        sendButtonEvent(event);

        // Stop pointer toplevel resizing sessions
        for (auto it = seat()->toplevelResizeSessions().begin(); it != seat()->toplevelResizeSessions().end();)
        {
            if ((*it)->triggeringEvent().type() != LEvent::Type::Touch)
                it = (*it)->stop();
            else
                it++;
        }

        // Stop pointer toplevel moving sessions
        for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
        {
            if ((*it)->triggeringEvent().type() != LEvent::Type::Touch)
                it = (*it)->stop();
            else
                it++;
        }

        // We stop sending events to the surface on which the left button was being held down
        setDraggingSurface(nullptr);

        if (!focus()->pointerConstraintEnabled() &&
            !focus()->inputRegion().contains(
                cursor()->pos().x() - focus()->rolePos().x(),
                cursor()->pos().y() - focus()->rolePos().y()))
        {
            setFocus(nullptr);
            cursor()->useDefault();
            cursor()->setVisible(true);
        }
    }
}
//! [pointerButtonEvent]

//! [pointerScrollEvent]
void LPointer::pointerScrollEvent(const LPointerScrollEvent &event)
{
    sendScrollEvent(event);
}
//! [pointerScrollEvent]


//! [pointerSwipeBeginEvent]
void LPointer::pointerSwipeBeginEvent(const LPointerSwipeBeginEvent &event)
{
    sendSwipeBeginEvent(event);
}
//! [pointerSwipeBeginEvent]

//! [pointerSwipeUpdateEvent]
void LPointer::pointerSwipeUpdateEvent(const LPointerSwipeUpdateEvent &event)
{
    sendSwipeUpdateEvent(event);
}
//! [pointerSwipeUpdateEvent]

//! [pointerSwipeEndEvent]
void LPointer::pointerSwipeEndEvent(const LPointerSwipeEndEvent &event)
{
    sendSwipeEndEvent(event);
}
//! [pointerSwipeEndEvent]

//! [pointerPinchBeginEvent]
void LPointer::pointerPinchBeginEvent(const LPointerPinchBeginEvent &event)
{
    sendPinchBeginEvent(event);
}
//! [pointerPinchBeginEvent]

//! [pointerPinchUpdateEvent]
void LPointer::pointerPinchUpdateEvent(const LPointerPinchUpdateEvent &event)
{
    sendPinchUpdateEvent(event);
}
//! [pointerPinchUpdateEvent]

//! [pointerPinchEndEvent]
void LPointer::pointerPinchEndEvent(const LPointerPinchEndEvent &event)
{
    sendPinchEndEvent(event);
}
//! [pointerPinchEndEvent]

//! [pointerHoldBeginEvent]
void LPointer::pointerHoldBeginEvent(const LPointerHoldBeginEvent &event)
{
    sendHoldBeginEvent(event);
}
//! [pointerHoldBeginEvent]

//! [pointerHoldEndEvent]
void LPointer::pointerHoldEndEvent(const LPointerHoldEndEvent &event)
{
    sendHoldEndEvent(event);
}
//! [pointerHoldEndEvent]

//! [setCursorRequest]
void LPointer::setCursorRequest(const LClientCursor &clientCursor)
{
    /* During a non-touch drag & drop session, the source client typically updates the cursor to
     * reflect the DND action (e.g., copy, move, not supported, etc.)
     */
    if (seat()->dnd()->dragging() && seat()->dnd()->triggeringEvent().type() != LEvent::Type::Touch)
    {
        if (seat()->dnd()->origin()->client() == clientCursor.client())
            cursor()->setCursor(clientCursor);

        return;
    }

    /* Allow the client to set the cursor only if one of its surfaces has pointer focus */
    if (focus() && focus()->client() == clientCursor.client())
        cursor()->setCursor(clientCursor);
}
//! [setCursorRequest]

//! [focusChanged]
void LPointer::focusChanged()
{
    /* No default implementation. */
}
//! [focusChanged]
