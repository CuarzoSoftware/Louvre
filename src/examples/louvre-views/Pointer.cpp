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
#include <LSurfaceView.h>
#include "Surface.h"

Pointer::Pointer(Params *params) : LPointer(params) {}

Compositor *Pointer::compositor() const
{
    return (Compositor*)LCompositor::compositor();
}

Surface *Pointer::focus()
{
    return (Surface*)focusSurface();
}

LPoint Pointer::viewLocalPos(LView *view)
{
    if ((view->scalingEnabled() || view->parentScalingEnabled()) && view->scalingVector().area() != 0.f)
        return (cursor()->posC() - view->posC()) / view->scalingVector();
    else
        return cursor()->posC() - view->posC();
}

void Pointer::pointerPosChangeEvent(Float32 x, Float32 y)
{
    cursor()->setPosC(LPointF(x,y));

    for (LView *sv : compositor()->hiddenCursorsLayer->children())
        ((LSurfaceView*)sv)->surface()->setPosC(cursor()->posC());

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
        sendMoveEventC(viewLocalPos(focus()->view));
        return;
    }

    // Find the first surface under the cursor
    LSurface *surface = nullptr;
    LView *view = compositor()->scene->viewAtC(cursor()->posC());

    if (view && view->type() == LView::Surface)
    {
        LSurfaceView *surfaceView = (LSurfaceView*)view;
        surface = surfaceView->surface();
    }

    if (!surface)
    {
        setFocusC(nullptr);
        cursor()->useDefault();
        cursor()->setVisible(true);
    }
    else
    {
        if (focusSurface() == surface)
            sendMoveEventC(viewLocalPos(view));
        else
            setFocusC(surface, viewLocalPos(view));
    }
}

void Pointer::pointerButtonEvent(Button button, ButtonState state)
{
    if (!focusSurface())
    {
        LSurface *surface = nullptr;
        LView *view = compositor()->scene->viewAtC(cursor()->posC());

        if (view && view->type() == LView::Surface)
        {
            LSurfaceView *surfaceView = (LSurfaceView*)view;
            surface = surfaceView->surface();
        }

        if (surface)
        {
            if (seat()->keyboard()->grabbingSurface() && seat()->keyboard()->grabbingSurface()->client() != surface->client())
            {
                seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
                dismissPopups();
            }

            if (!seat()->keyboard()->focusSurface() || !surface->isSubchildOf(seat()->keyboard()->focusSurface()))
                seat()->keyboard()->setFocus(surface);

            setFocusC(surface, viewLocalPos(view));
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
        stopResizingToplevel();
        stopMovingToplevel();

        // We stop sending events to the surface on which the left button was being held down
        setDragginSurface(nullptr);

        if (!focus()->view->inputRegionC()->containsPoint(viewLocalPos(focus()->view)))
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            setFocusC(nullptr);
            cursor()->useDefault();
            cursor()->setVisible(true);
        }
    }
}
