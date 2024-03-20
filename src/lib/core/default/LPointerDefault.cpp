#include <LLog.h>
#include <LPointer.h>
#include <LSeat.h>
#include <LCompositor.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LClient.h>
#include <LPopupRole.h>
#include <LTime.h>
#include <LKeyboard.h>
#include <LDND.h>
#include <LDNDIconRole.h>
#include <LCursorRole.h>
#include <LPointerMoveEvent.h>
#include <LPointerButtonEvent.h>
#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
#include <LClientCursor.h>

using namespace Louvre;

//! [pointerMoveEvent]
void LPointer::pointerMoveEvent(const LPointerMoveEvent &event)
{
    // Update the cursor position
    cursor()->move(event.delta().x(), event.delta().y());

    // Schedule repaint on outputs that intersect with the cursor if hardware composition is not supported.
    cursor()->repaintOutputs(true);

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
            session->updateDragPoint(cursor()->pos());
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
            session->updateDragPoint(cursor()->pos());
            session->toplevel()->surface()->repaintOutputs();

            if (session->toplevel()->maximized())
                session->toplevel()->configure(session->toplevel()->pendingStates() &~ LToplevelRole::Maximized);
        }
    }

    if (activeMoving)
        return;

    // If a surface had the left pointer button held down
    if (draggingSurface())
    {
        event.localPos = cursor()->pos() - draggingSurface()->rolePos();
        sendMoveEvent(event);
        return;
    }

    // Find the first surface under the cursor
    LSurface *surface { surfaceAt(cursor()->pos()) };

    if (surface)
    {
        event.localPos = cursor()->pos() - surface->rolePos();

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
            seat()->dnd()->setFocus(nullptr, LPointF());
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
    const bool activeDND { seat()->dnd()->dragging() && seat()->dnd()->triggeringEvent().type() != LEvent::Type::Touch };

    if (activeDND)
    {
        if (event.state() == LPointerButtonEvent::Released && event.button() == LPointerButtonEvent::Left)
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
            cursor()->setCursor(surface->client()->lastCursorRequest());
            seat()->keyboard()->setFocus(surface);
            setFocus(surface);
            sendButtonEvent(event);

            if (!surface->popup())
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
        // We save the pointer focus surface to continue sending events to it even when the cursor
        // is outside of it (while the left button is being held down)
        setDraggingSurface(focus());

        if (!seat()->keyboard()->focus() || !focus()->isSubchildOf(seat()->keyboard()->focus()))
            seat()->keyboard()->setFocus(focus());

        if (focus()->toplevel() && !focus()->toplevel()->activated())
            focus()->toplevel()->configure(focus()->toplevel()->pendingStates() | LToplevelRole::Activated);

        if (!focus()->popup())
            seat()->dismissPopups();

        sendButtonEvent(event);

        if (focus() == compositor()->surfaces().back())
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

        if (!focus()->inputRegion().containsPoint(cursor()->pos() - focus()->rolePos()))
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
    if (seat()->dnd()->dragging())
    {
        if (seat()->dnd()->origin()->client() == clientCursor.client())
            cursor()->setCursor(clientCursor);

        return;
    }

    if (focus() && focus()->client() == clientCursor.client())
        cursor()->setCursor(clientCursor);
}
//! [setCursorRequest]
