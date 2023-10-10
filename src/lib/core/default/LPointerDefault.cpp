#include <LLog.h>
#include <LPointer.h>
#include <LSeat.h>
#include <LCompositor.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LPopupRole.h>
#include <LTime.h>
#include <LKeyboard.h>
#include <LDNDManager.h>
#include <LDNDIconRole.h>
#include <LCursorRole.h>

using namespace Louvre;

//! [pointerMoveEvent]
void LPointer::pointerMoveEvent(Float32 dx, Float32 dy)
{
    /* For simplification, we manage the event in pointerPosChangeEvent().
     * Input backends emit either pointerMoveEvent() or pointerPosChangeEvent(),
     * but not both. */
    pointerPosChangeEvent(cursor()->pos().x() + dx,
                          cursor()->pos().y() + dy);
}
//! [pointerMoveEvent]

//! [pointerPosChangeEvent]
void LPointer::pointerPosChangeEvent(Float32 x, Float32 y)
{
    cursor()->setPos(LPointF(x, y));

    // Repaint outputs that intersect with the cursor if hardware composition is not supported.
    cursor()->repaintOutputs(true);

    // Update the drag & drop icon position
    if (seat()->dndManager()->icon())
    {
        seat()->dndManager()->icon()->surface()->setPos(cursor()->pos());
        seat()->dndManager()->icon()->surface()->repaintOutputs();
    }

    if (resizingToplevel())
    {
        updateResizingToplevelSize(cursor()->pos());
        return;
    }

    if (movingToplevel())
    {
        updateMovingToplevelPos(cursor()->pos());

        movingToplevel()->surface()->repaintOutputs();

        if (movingToplevel()->maximized())
            movingToplevel()->configure(movingToplevel()->states() &~ LToplevelRole::Maximized);

        return;
    }

    // If we are in a drag & drop session, we call setDraggingSurface(nullptr)
    // to prevent the current surface from retaining focus.
    if (seat()->dndManager()->dragging())
        setDraggingSurface(nullptr);

    // If a surface had the left pointer button held down
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
//! [pointerPosChangeEvent]

//! [pointerButtonEvent]
void LPointer::pointerButtonEvent(Button button, ButtonState state)
{
    if (state == Released && button == Left)
        seat()->dndManager()->drop();

    if (!focusSurface())
    {
        LSurface *surface = surfaceAt(cursor()->pos());

        if (surface)
        {
            seat()->keyboard()->setFocus(surface);
            setFocus(surface);
            sendButtonEvent(button, state);

            if (surface->popup())
                dismissPopups();
        }
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
        // We save the pointer focus surface to continue sending events to it even when the cursor
        // is outside of it (while the left button is being held down)
        setDraggingSurface(focusSurface());

        seat()->keyboard()->setFocus(focusSurface());

        if (focusSurface()->toplevel() && !focusSurface()->toplevel()->activated())
            focusSurface()->toplevel()->configure(focusSurface()->toplevel()->states() | LToplevelRole::Activated);

        if (!focusSurface()->popup())
            dismissPopups();

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
        stopResizingToplevel();
        stopMovingToplevel();

        // We stop sending events to the surface on which the left button was being held down
        setDraggingSurface(nullptr);

        if (!focusSurface()->inputRegion().containsPoint(cursor()->pos() - focusSurface()->rolePos()))
        {
            setFocus(nullptr);
            cursor()->useDefault();
            cursor()->setVisible(true);
        }
    }
}
//! [pointerButtonEvent]

//! [pointerAxisEvent]
void LPointer::pointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, AxisSource source)
{
    // Invert the scroll axis for natural scrolling
    sendAxisEvent(-axisX, -axisY, -discreteX, -discreteY, source);
}
//! [pointerAxisEvent]

//! [setCursorRequest]
void LPointer::setCursorRequest(LCursorRole *cursorRole)
{
    if (cursorRole)
    {
        cursor()->setTextureB(
                    cursorRole->surface()->texture(),
                    cursorRole->hotspotB());

        cursor()->setVisible(true);
    }
    // If `nullptr` is provided, it indicates that the client intends to hide the cursor.
    else
        cursor()->setVisible(false);
}
//! [setCursorRequest]
