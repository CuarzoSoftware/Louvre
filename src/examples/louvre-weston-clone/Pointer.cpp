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

Pointer::Pointer(Params *params) : LPointer(params)
{
    timespec ts = LTime::ns();
    lastEventMs = float(ts.tv_nsec)/100000.f + float(ts.tv_sec)*10000.f;
}

void Pointer::pointerMoveEvent(float dx, float dy)
{
    timespec ts = LTime::ns();
    float ct = float(ts.tv_nsec)/100000.f + float(ts.tv_sec)*10000.f;
    float dt =  ct - lastEventMs;
    lastEventMs = ct;
    float speed = 1.4f;

    if (dt >= 1.f)
        speed += 0.5f*sqrt(dx*dx + dy*dy)/dt;

    pointerPosChangeEvent(
        cursor()->pos().x() + dx * speed,
        cursor()->pos().y() + dy * speed);
}

void Pointer::pointerPosChangeEvent(Float32 x, Float32 y)
{
    cursor()->setPos(LPointF(x, y));

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

    // Update the Toplevel size (if there was one being resized)
    if (resizingToplevel())
    {
        updateResizingToplevelSize();
        return;
    }

    // Update the Toplevel pos (if there was one being moved interactively)
    if (movingToplevel())
    {
        updateMovingToplevelPos();

        movingToplevel()->surface()->repaintOutputs();

        if (movingToplevel()->maximized())
            movingToplevel()->configure(movingToplevel()->states() &~ LToplevelRole::Maximized);

        return;
    }

    // DO NOT GET CONFUSED! If we are in a drag & drop session, we call setDragginSurface(NULL) in case there is a surface being dragged.
    if (seat()->dndManager()->dragging())
        setDragginSurface(nullptr);

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
        if (focusSurface() == surface)
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
            {
                system("weston-terminal");
                exit(0);
            }
        }
    }

    if (!focusSurface() && !pointerOverTerminalIcon)
    {
        LSurface *surface = surfaceAt(cursor()->pos());

        if (surface)
        {
            if (seat()->keyboard()->grabbingSurface() && seat()->keyboard()->grabbingSurface()->client() != surface->client())
            {
                seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
                dismissPopups();
            }

            if (!seat()->keyboard()->focusSurface() || !surface->isSubchildOf(seat()->keyboard()->focusSurface()))
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
        setDragginSurface(focusSurface());

        if (seat()->keyboard()->grabbingSurface() && seat()->keyboard()->grabbingSurface()->client() != focusSurface()->client())
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            dismissPopups();
        }

        if (!focusSurface()->popup())
            dismissPopups();

        if (!seat()->keyboard()->focusSurface() || !focusSurface()->isSubchildOf(seat()->keyboard()->focusSurface()))
            seat()->keyboard()->setFocus(focusSurface());

        if (focusSurface()->toplevel() && !focusSurface()->toplevel()->activated())
            focusSurface()->toplevel()->configure(focusSurface()->toplevel()->states() | LToplevelRole::Activated);

        // Raise surface
        if (focusSurface() == compositor()->surfaces().back())
            return;

        if (focusSurface()->parent())
            focusSurface()->topmostParent()->raise();
        else
            focusSurface()->raise();
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
        setDragginSurface(nullptr);

        if (!focusSurface()->inputRegion().containsPoint(cursor()->pos() - focusSurface()->rolePos()))
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            setFocus(nullptr);
            cursor()->useDefault();
            cursor()->setVisible(true);
        }
    }
}
