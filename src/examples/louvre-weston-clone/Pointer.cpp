#include "Pointer.h"
#include "Compositor.h"
#include "Output.h"
#include <LDNDIconRole.h>
#include <LDNDManager.h>
#include <LSeat.h>
#include <LTime.h>
#include <LCursor.h>
#include <cstdio>
#include <math.h>
#include <unistd.h>

Pointer::Pointer(Params *params) : LPointer(params) {}

void Pointer::pointerMoveEvent(Float32 x, Float32 y, bool absolute)
{
    if (absolute)
        cursor()->setPos(x, y);
    else
        cursor()->move(x, y);

    Output *cursorOutput = (Output*)cursor()->output();
    Compositor *c = (Compositor*)compositor();

    bool pointerOverTerminalIcon = cursorOutput->terminalIconRect.containsPoint(cursor()->pos());

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

    // Repaint cursor outputs if hardware composition is not supported
    for (LOutput *output : cursor()->intersectedOutputs())
    {
        if (!cursor()->hasHardwareSupport(output))
            output->repaint();
    }

    // Update the drag & drop icon (if there was one)
    if (seat()->dndManager()->icon())
    {
        seat()->dndManager()->icon()->surface()->setPos(cursor()->pos());
        seat()->dndManager()->icon()->surface()->repaintOutputs();
    }

    // Update the toplevel size (if there was one being resized)
    if (resizingToplevel())
    {
        updateResizingToplevelSize(cursor()->pos());
        return;
    }

    // Update the toplevel pos (if there was one being moved interactively)
    if (movingToplevel())
    {
        updateMovingToplevelPos(cursor()->pos());

        movingToplevel()->surface()->repaintOutputs();

        if (movingToplevel()->maximized())
            movingToplevel()->configure(movingToplevel()->states() &~ LToplevelRole::Maximized);

        return;
    }

    // DO NOT GET CONFUSED! If we are in a drag & drop session, we call setDragginSurface(NULL) in case there is a surface being dragged.
    if (seat()->dndManager()->dragging())
        setDraggingSurface(nullptr);

    // If there was a surface holding the left pointer button
    if (draggingSurface())
    {
        sendMoveEvent();
        return;
    }

    // Find the first surface under the cursor
    LSurface *surface = surfaceAt(cursor()->pos());

    if (!surface)
    {
        setFocus(nullptr);

        if (!pointerOverTerminalIcon)
            cursor()->useDefault();

        cursor()->setVisible(true);
    }
    else
    {
        if (focus() == surface)
            sendMoveEvent();
        else
            setFocus(surface);
    }
}

void Pointer::pointerButtonEvent(Button button, ButtonState state)
{
    Output *cursorOutput = (Output*)cursor()->output();

    bool pointerOverTerminalIcon = cursorOutput->terminalIconRect.containsPoint(cursor()->pos());

    if (state == Released && button == Left)
    {
        seat()->dndManager()->drop();

        if (pointerOverTerminalIcon)
        {
            if (fork() == 0)
                exit(system("weston-terminal"));
        }
    }

    if (!focus() && !pointerOverTerminalIcon)
    {
        LSurface *surface = surfaceAt(cursor()->pos());

        if (surface)
        {
            if (seat()->keyboard()->grabbingSurface() && seat()->keyboard()->grabbingSurface()->client() != surface->client())
            {
                seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
                dismissPopups();
            }

            if (!seat()->keyboard()->focus() || !surface->isSubchildOf(seat()->keyboard()->focus()))
                seat()->keyboard()->setFocus(surface);

            setFocus(surface);
            sendButtonEvent(button, state);
        }
        // If no surface under the cursor
        else
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            seat()->keyboard()->setFocus(nullptr);
            dismissPopups();
        }

        return;
    }

    sendButtonEvent(button, state);

    if (button != Left)
        return;

    // Left button pressed
    if (state == Pressed)
    {
        if (pointerOverTerminalIcon)
        {
            cursorOutput->newDamage.addRect(cursorOutput->terminalIconRect);
            cursorOutput->terminalIconAlpha = 0.6f;
            cursorOutput->repaint();
            return;
        }

        /* We save the pointer focus surface in order to continue sending events to it even when the cursor
         * is outside of it (while the left button is being held down)*/
        setDraggingSurface(focus());

        if (seat()->keyboard()->grabbingSurface() && seat()->keyboard()->grabbingSurface()->client() != focus()->client())
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            dismissPopups();
        }

        if (!focus()->popup())
            dismissPopups();

        if (!seat()->keyboard()->focus() || !focus()->isSubchildOf(seat()->keyboard()->focus()))
            seat()->keyboard()->setFocus(focus());

        if (focus()->toplevel() && !focus()->toplevel()->activated())
            focus()->toplevel()->configure(focus()->toplevel()->states() | LToplevelRole::Activated);

        // Raise surface
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

        stopResizingToplevel();
        stopMovingToplevel();

        // We stop sending events to the surface on which the left button was being held down
        setDraggingSurface(nullptr);

        if (!focus()->inputRegion().containsPoint(cursor()->pos() - focus()->rolePos()))
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            setFocus(nullptr);
            cursor()->useDefault();
            cursor()->setVisible(true);
        }
    }
}
