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
void LPointer::pointerMoveEvent(Float32 x, Float32 y, bool absolute)
{
    if (absolute)
        cursor()->setPos(x, y);
    else
        cursor()->move(x, y);

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
            movingToplevel()->configure(movingToplevel()->pendingStates() &~ LToplevelRole::Maximized);

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
    LSurface *surface { surfaceAt(cursor()->pos()) };

    if (surface)
    {
        if (focus() == surface)
            sendMoveEvent();
        else
            setFocus(surface);
    }
    else
    {
        setFocus(nullptr);
        cursor()->useDefault();
        cursor()->setVisible(true);
    }
}
//! [pointerMoveEvent]

//! [pointerButtonEvent]
void LPointer::pointerButtonEvent(Button button, ButtonState state)
{
    if (state == Released && button == Left)
        seat()->dndManager()->drop();

    if (!focus())
    {
        LSurface *surface { surfaceAt(cursor()->pos()) };

        if (surface)
        {
            seat()->keyboard()->setFocus(surface);
            setFocus(surface);
            sendButtonEvent(button, state);

            if (!surface->popup())
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
        setDraggingSurface(focus());

        if (!seat()->keyboard()->focus() || !focus()->isSubchildOf(seat()->keyboard()->focus()))
            seat()->keyboard()->setFocus(focus());

        if (focus()->toplevel() && !focus()->toplevel()->activated())
            focus()->toplevel()->configure(focus()->toplevel()->pendingStates() | LToplevelRole::Activated);

        if (!focus()->popup())
            dismissPopups();

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
        stopResizingToplevel();
        stopMovingToplevel();

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
