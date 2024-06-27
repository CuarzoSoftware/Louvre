#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
#include <LPointerMoveEvent.h>
#include <LDNDIconRole.h>
#include <LDND.h>
#include <LKeyboard.h>
#include <LClient.h>
#include <LSeat.h>
#include <LTime.h>
#include <LCursor.h>
#include <LLauncher.h>
#include <unistd.h>

#include "Pointer.h"
#include "Compositor.h"
#include "Output.h"

void Pointer::pointerMoveEvent(const LPointerMoveEvent &event)
{
    Output *cursorOutput { static_cast<Output*>(cursor()->output()) };
    Compositor *c { static_cast<Compositor*>(compositor()) };
    bool pointerOverTerminalIcon { false };

    // Update the cursor position
    cursor()->move(event.delta().x(), event.delta().y());

    bool pointerConstrained { false };

    if (focus())
    {
        LPointF pos { focus()->rolePos() };

        if (focus()->pointerConstraintMode() != LSurface::PointerConstraintMode::Free)
        {
            if (focus()->pointerConstraintRegion().containsPoint(cursor()->pos() - pos))
                focus()->enablePointerConstraint(true);
        }

        if (focus()->pointerConstraintEnabled())
        {
            if (focus()->pointerConstraintMode() == LSurface::PointerConstraintMode::Lock)
            {
                if (focus()->lockedPointerPosHint().x() >= 0.f)
                    cursor()->setPos(pos + focus()->lockedPointerPosHint());
                else
                {
                    cursor()->move(-event.delta().x(), -event.delta().y());

                    const LPointF closestPoint {
                        focus()->pointerConstraintRegion().closestPointFrom(cursor()->pos() - pos, 0.5f)
                    };

                    cursor()->setPos(pos + closestPoint);
                }
            }
            else
            {
                const LPointF closestPoint {
                    focus()->pointerConstraintRegion().closestPointFrom(cursor()->pos() - pos, 0.5f)
                };

                cursor()->setPos(pos + closestPoint);
            }

            pointerConstrained = true;
        }
    }

    // Schedule repaint on outputs that intersect with the cursor if hardware composition is not supported.
    cursor()->repaintOutputs(true);

    // Update terminal icon
    if (cursorOutput)
    {
        pointerOverTerminalIcon = cursorOutput->terminalIconRect.containsPoint(cursor()->pos());

        if (pointerOverTerminalIcon)
        {
            if (cursorOutput->terminalIconAlpha == 1.0f)
            {
                cursorOutput->terminalIconRect += LRect(-1,-1, 2, 2);
                cursorOutput->newDamage.addRect(cursorOutput->terminalIconRect);
                cursorOutput->terminalIconAlpha = 0.9f;
                cursorOutput->repaint();
            }

            if (c->pointerCursor)
                cursor()->setTextureB(c->pointerCursor->texture(), c->pointerCursor->hotspotB());
        }
        else
        {
            if (cursorOutput->terminalIconAlpha != 1.0f)
            {
                cursorOutput->newDamage.addRect(cursorOutput->terminalIconRect);
                cursorOutput->terminalIconRect += LRect(1, 1, -2, -2);
                cursorOutput->terminalIconAlpha = 1.0f;
                cursorOutput->repaint();
            }
        }
    }

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
                session->toplevel()->configureState(session->toplevel()->pendingConfiguration().state &~ LToplevelRole::Maximized);
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
    LSurface *surface { pointerConstrained ? focus() : surfaceAt(cursor()->pos()) };

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

            if (!pointerOverTerminalIcon)
                cursor()->setCursor(surface->client()->lastCursorRequest());
        }
    }
    else
    {
        setFocus(nullptr);

        if (!pointerOverTerminalIcon && !activeDND)
        {
            cursor()->useDefault();
            cursor()->setVisible(true);
        }

        if (activeDND)
            seat()->dnd()->setFocus(nullptr, LPointF());
    }
}

void Pointer::pointerButtonEvent(const LPointerButtonEvent &event)
{
    Output *cursorOutput { static_cast<Output*>(cursor()->output()) };
    bool pointerOverTerminalIcon { false };
    const bool activeDND { seat()->dnd()->dragging() && seat()->dnd()->triggeringEvent().type() != LEvent::Type::Touch };

    if (cursorOutput)
        pointerOverTerminalIcon = cursorOutput->terminalIconRect.containsPoint(cursor()->pos());

    if (event.state() == LPointerButtonEvent::Released && event.button() == LPointerButtonEvent::Left)
        if (pointerOverTerminalIcon)
            LLauncher::launch("weston-terminal");

    if (activeDND)
    {
        if (event.state() == LPointerButtonEvent::Released && event.button() == LPointerButtonEvent::Left)
            seat()->dnd()->drop();
        seat()->keyboard()->setFocus(nullptr);
        setFocus(nullptr);
        setDraggingSurface(nullptr);
        return;
    }

    if (!focus() && !pointerOverTerminalIcon)
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
        if (pointerOverTerminalIcon)
        {
            cursorOutput->newDamage.addRect(cursorOutput->terminalIconRect);
            cursorOutput->terminalIconAlpha = 0.6f;
            cursorOutput->repaint();
            return;
        }

        // We save the pointer focus surface to continue sending events to it even when the cursor
        // is outside of it (while the left button is being held down)
        setDraggingSurface(focus());

        if (!seat()->keyboard()->focus() || !focus()->isSubchildOf(seat()->keyboard()->focus()))
            seat()->keyboard()->setFocus(focus());

        if (focus()->toplevel() && !focus()->toplevel()->activated())
            focus()->toplevel()->configureState(focus()->toplevel()->pendingConfiguration().state | LToplevelRole::Activated);

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
        if (pointerOverTerminalIcon)
        {
            cursorOutput->newDamage.addRect(cursorOutput->terminalIconRect);
            cursorOutput->terminalIconAlpha = 0.9f;
            cursorOutput->repaint();
            return;
        }

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

        if (!focus()->pointerConstraintEnabled() && !focus()->inputRegion().containsPoint(cursor()->pos() - focus()->rolePos()))
        {
            setFocus(nullptr);
            cursor()->useDefault();
            cursor()->setVisible(true);
        }
    }
}
