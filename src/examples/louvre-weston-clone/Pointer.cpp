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
        cursor()->posC().x() + dx*speed,
        cursor()->posC().y() + dy*speed);
}

void Pointer::pointerPosChangeEvent(Float32 x, Float32 y)
{
    cursor()->setPosC(LPointF(x,y));

    Output *cursorOutput = (Output*)cursor()->output();
    Compositor *c = (Compositor*)compositor();

    bool pointerOverTerminalIcon = cursorOutput->terminalIconRectC.containsPoint(cursor()->posC());

    if (pointerOverTerminalIcon)
    {
        if (cursorOutput->terminalIconAlpha == 1.0f)
        {
            cursorOutput->terminalIconRectC += LRect(-1,-1, 2, 2);
            cursorOutput->newDamage.addRect(cursorOutput->terminalIconRectC);
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
            cursorOutput->newDamage.addRect(cursorOutput->terminalIconRectC);
            cursorOutput->terminalIconRectC += LRect(1, 1, -2, -2);
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
        seat()->dndManager()->icon()->surface()->setPosC(cursor()->posC());
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
            movingToplevel()->configureC(movingToplevel()->states() &~ LToplevelRole::Maximized);

        return;
    }

    // DO NOT GET CONFUSED! If we are in a drag & drop session, we call setDragginSurface(NULL) in case there is a surface being dragged.
    if (seat()->dndManager()->dragging())
        setDragginSurface(nullptr);

    // If there was a surface holding the left pointer button
    if (draggingSurface())
    {
        sendMoveEventC();
        return;
    }

    // Find the first surface under the cursor
    LSurface *surface = surfaceAtC(cursor()->posC());

    if (!surface)
    {
        setFocusC(nullptr);

        if (!pointerOverTerminalIcon)
            cursor()->useDefault();

        cursor()->setVisible(true);
    }
    else
    {
        if (focusSurface() == surface)
            sendMoveEventC();
        else
            setFocusC(surface);
    }
}

void Pointer::pointerButtonEvent(Button button, ButtonState state)
{
    Output *cursorOutput = (Output*)cursor()->output();

    bool pointerOverTerminalIcon = cursorOutput->terminalIconRectC.containsPoint(cursor()->posC());

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
        LSurface *surface = surfaceAtC(cursor()->posC());

        if (surface)
        {
            seat()->keyboard()->setFocus(surface);
            setFocusC(surface);
            sendButtonEvent(button,state);

            if (surface->popup())
                dismissPopups();
        }
        // If no surface under the cursor
        else
        {
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
            cursorOutput->newDamage.addRect(cursorOutput->terminalIconRectC);
            cursorOutput->terminalIconAlpha = 0.6f;
            cursorOutput->repaint();
            return;
        }

        /* We save the pointer focus surface in order to continue sending events to it even when the cursor
         * is outside of it (while the left button is being held down)*/
        setDragginSurface(focusSurface());

        if (!seat()->keyboard()->focusSurface() || (focusSurface()->client() != seat()->keyboard()->focusSurface()->client()))
            seat()->keyboard()->setFocus(focusSurface());

        if (focusSurface()->toplevel() && !focusSurface()->toplevel()->activated())
            focusSurface()->toplevel()->configureC(focusSurface()->toplevel()->states() | LToplevelRole::Activated);

        // Raise surface
        if (focusSurface() == compositor()->surfaces().back())
            return;

        if (focusSurface()->parent())
            compositor()->raiseSurface(focusSurface()->topmostParent());
        else
            compositor()->raiseSurface(focusSurface());
    }
    // Left button released
    else
    {
        if (pointerOverTerminalIcon)
        {
            cursorOutput->newDamage.addRect(cursorOutput->terminalIconRectC);
            cursorOutput->terminalIconAlpha = 0.9f;
            cursorOutput->repaint();
            return;
        }

        stopResizingToplevel();
        stopMovingToplevel();

        // We stop sending events to the surface on which the left button was being held down
        setDragginSurface(nullptr);

        if (!focusSurface()->inputRegionC().containsPoint(cursor()->posC() - focusSurface()->rolePosC()))
        {
            setFocusC(nullptr);
            cursor()->useDefault();
            cursor()->setVisible(true);
        }
    }
}
