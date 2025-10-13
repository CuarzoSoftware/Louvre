#include <CZ/Core/Utils/CZRegionUtils.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Roles/LPopupRole.h>
#include <CZ/Core/CZTime.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/Cursor/LCursorSource.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Louvre/Roles/LToplevelMoveSession.h>
#include <CZ/Louvre/Roles/LToplevelResizeSession.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>

using namespace CZ;

//! [pointerMoveEvent]
void LPointer::pointerMoveEvent(const CZPointerMoveEvent &event)
{
    // Update the cursor position
    cursor()->move(event.delta.x(), event.delta.y());

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
                    cursor()->move(-event.delta.x(), -event.delta.y());

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
    const bool activeDND { seat()->dnd()->dragging() && !seat()->dnd()->triggeringEvent().isTouchEvent() };

    if (activeDND)
    {
        if (seat()->dnd()->icon())
        {
            seat()->dnd()->icon()->surface()->setPos(cursor()->pos());
            seat()->dnd()->icon()->surface()->repaintOutputs();
            cursor()->setSource(seat()->dnd()->icon()->surface()->client()->cursor());
        }

        seat()->keyboard()->setFocus(nullptr);
        setDraggingSurface(nullptr);
        setFocus(nullptr);
    }

    bool activeResizing { false };

    for (LToplevelResizeSession *session : seat()->toplevelResizeSessions())
    {
        if (!session->triggeringEvent().isTouchEvent())
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
        if (!session->triggeringEvent().isTouchEvent())
        {
            activeMoving = true;
            session->updateDragPoint(SkIPoint(cursor()->pos().x(), cursor()->pos().y()));
            session->toplevel()->surface()->repaintOutputs();

            if (session->toplevel()->isMaximized())
                session->toplevel()->configureState(session->toplevel()->pendingConfiguration().windowState &~ CZWinMaximized);
        }
    }

    if (activeMoving)
        return;

    // If a surface had the left pointer button held down
    if (draggingSurface())
    {
        event.pos = cursor()->pos() - SkPoint(draggingSurface()->rolePos().x(), draggingSurface()->rolePos().y());
        sendMoveEvent(event);
        return;
    }

    // Find the first surface under the cursor
    LSurface *surface { pointerConstrained ? focus() : seat()->surfaceAt(cursor()->pos()) };

    if (surface)
    {
        if (sessionLocked && surface->client() != sessionLockManager()->client())
            return;

        event.pos = cursor()->pos() - SkPoint(surface->rolePos().x(), surface->rolePos().y());

        if (activeDND)
        {
            if (seat()->dnd()->focus() == surface)
                seat()->dnd()->sendMoveEvent(event.pos, event.ms);
            else
                seat()->dnd()->setFocus(surface, event.pos);
        }
        else
        {
            if (focus() == surface)
                sendMoveEvent(event);
            else
                setFocus(surface, event.pos);
        }

        cursor()->setSource(surface->client()->cursor());
    }
    else
    {
        if (activeDND)
            seat()->dnd()->setFocus(nullptr, SkPoint());
        else
        {
            setFocus(nullptr);
            cursor()->setSource({});
            cursor()->setVisible(true);
        }
    }
}
//! [pointerMoveEvent]

//! [pointerButtonEvent]
void LPointer::pointerButtonEvent(const CZPointerButtonEvent &event)
{
    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };
    const bool activeDND { seat()->dnd()->dragging() && !seat()->dnd()->triggeringEvent().isTouchEvent() };

    if (activeDND)
    {
        if (!event.pressed && event.button == BTN_LEFT)
            seat()->dnd()->drop();

        seat()->keyboard()->setFocus(nullptr);
        setFocus(nullptr);
        setDraggingSurface(nullptr);
        return;
    }

    if (!focus())
    {
        LSurface *surface { seat()->surfaceAt(cursor()->pos()) };

        if (surface)
        {
            if (sessionLocked && surface->client() != sessionLockManager()->client())
                return;

            cursor()->setSource(surface->client()->cursor());
            seat()->keyboard()->setFocus(surface);
            setFocus(surface);
            sendButtonEvent(event);

            if (!surface->popup() && !surface->isSubchildOf(LSurface::Popup))
                seat()->dismissPopups();
        }
        else
        {
            seat()->keyboard()->setFocus(nullptr);
            seat()->dismissPopups();
        }

        return;
    }

    if (event.button != BTN_LEFT)
    {
        sendButtonEvent(event);
        return;
    }

    // Left button pressed
    if (event.pressed)
    {
        // Keep a ref to continue sending it events after the cursor
        // leaves, if the left button remains pressed
        setDraggingSurface(focus());

        // Most apps close popups when they get keyboard focus,
        // probably because the parent loses it
        if (!focus()->popup() && !focus()->isSubchildOf(LSurface::Popup))
        {
            seat()->keyboard()->setFocus(focus());

            // Pointer focus may have changed within LKeyboard::focusChanged()
            if (!focus())
                return;
        }

        sendButtonEvent(event);

        if (focus()->toplevel() && !focus()->toplevel()->isActivated())
            focus()->toplevel()->configureState(focus()->toplevel()->pendingConfiguration().windowState | CZWinActivated);

        if (!focus()->popup() && !focus()->isSubchildOf(LSurface::Popup))
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
            if (!(*it)->triggeringEvent().isTouchEvent())
                it = (*it)->stop();
            else
                it++;
        }

        // Stop pointer toplevel moving sessions
        for (auto it = seat()->toplevelMoveSessions().begin(); it != seat()->toplevelMoveSessions().end();)
        {
            if (!(*it)->triggeringEvent().isTouchEvent())
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
            cursor()->setSource({});
            cursor()->setVisible(true);
        }
    }
}
//! [pointerButtonEvent]

//! [pointerScrollEvent]
void LPointer::pointerScrollEvent(const CZPointerScrollEvent &event)
{
    sendScrollEvent(event);
}
//! [pointerScrollEvent]


//! [pointerSwipeBeginEvent]
void LPointer::pointerSwipeBeginEvent(const CZPointerSwipeBeginEvent &event)
{
    sendSwipeBeginEvent(event);
}
//! [pointerSwipeBeginEvent]

//! [pointerSwipeUpdateEvent]
void LPointer::pointerSwipeUpdateEvent(const CZPointerSwipeUpdateEvent &event)
{
    sendSwipeUpdateEvent(event);
}
//! [pointerSwipeUpdateEvent]

//! [pointerSwipeEndEvent]
void LPointer::pointerSwipeEndEvent(const CZPointerSwipeEndEvent &event)
{
    sendSwipeEndEvent(event);
}
//! [pointerSwipeEndEvent]

//! [pointerPinchBeginEvent]
void LPointer::pointerPinchBeginEvent(const CZPointerPinchBeginEvent &event)
{
    sendPinchBeginEvent(event);
}
//! [pointerPinchBeginEvent]

//! [pointerPinchUpdateEvent]
void LPointer::pointerPinchUpdateEvent(const CZPointerPinchUpdateEvent &event)
{
    sendPinchUpdateEvent(event);
}
//! [pointerPinchUpdateEvent]

//! [pointerPinchEndEvent]
void LPointer::pointerPinchEndEvent(const CZPointerPinchEndEvent &event)
{
    sendPinchEndEvent(event);
}
//! [pointerPinchEndEvent]

//! [pointerHoldBeginEvent]
void LPointer::pointerHoldBeginEvent(const CZPointerHoldBeginEvent &event)
{
    sendHoldBeginEvent(event);
}
//! [pointerHoldBeginEvent]

//! [pointerHoldEndEvent]
void LPointer::pointerHoldEndEvent(const CZPointerHoldEndEvent &event)
{
    sendHoldEndEvent(event);
}
//! [pointerHoldEndEvent]

//! [setCursorRequest]
void LPointer::setCursorRequest(std::shared_ptr<LCursorSource> source) noexcept
{
    /* During a non-touch drag & drop session, the source client typically updates the cursor to
     * reflect the DND action (e.g., copy, move, not supported, etc.)
     */
    if (seat()->dnd()->dragging() && !seat()->dnd()->triggeringEvent().isTouchEvent())
    {
        if (seat()->dnd()->origin()->client() == source->client())
            cursor()->setSource(source);

        return;
    }

    /* Allow the client to set the cursor only if one of its surfaces has pointer focus */
    if (focus() && focus()->client() == source->client())
        cursor()->setSource(source);
}
//! [setCursorRequest]

//! [focusChanged]
void LPointer::focusChanged()
{
    /* No default implementation. */
}
//! [focusChanged]

void LPointer::updateButtons(const CZPointerButtonEvent &e) noexcept
{
    if (e.pressed)
        pressedButtons.emplace(e.button);
    else
        pressedButtons.erase(e.button);
}

bool LPointer::event(const CZEvent &e) noexcept
{
    switch (e.type())
    {
    case CZEvent::Type::PointerMove:
        pointerMoveEvent((const CZPointerMoveEvent&)e);
        break;
    case CZEvent::Type::PointerScroll:
        pointerScrollEvent((const CZPointerScrollEvent&)e);
        break;
    case CZEvent::Type::PointerSwipeUpdate:
        pointerSwipeUpdateEvent((const CZPointerSwipeUpdateEvent&)e);
        break;
    case CZEvent::Type::PointerPinchUpdate:
        pointerPinchUpdateEvent((const CZPointerPinchUpdateEvent&)e);
        break;
    case CZEvent::Type::PointerButton:
        updateButtons((const CZPointerButtonEvent&)e);
        pointerButtonEvent((const CZPointerButtonEvent&)e);
        break;
    case CZEvent::Type::PointerSwipeBegin:
        pointerSwipeBeginEvent((const CZPointerSwipeBeginEvent&)e);
        break;
    case CZEvent::Type::PointerSwipeEnd:
        pointerSwipeEndEvent((const CZPointerSwipeEndEvent&)e);
        break;
    case CZEvent::Type::PointerPinchBegin:
        pointerPinchBeginEvent((const CZPointerPinchBeginEvent&)e);
        break;
    case CZEvent::Type::PointerPinchEnd:
        pointerPinchEndEvent((const CZPointerPinchEndEvent&)e);
        break;
    case CZEvent::Type::PointerHoldBegin:
        pointerHoldBeginEvent((const CZPointerHoldBeginEvent&)e);
        break;
    case CZEvent::Type::PointerHoldEnd:
        pointerHoldEndEvent((const CZPointerHoldEndEvent&)e);
        break;
    default:
        break;
    }

    return LFactoryObject::event(e);
}
