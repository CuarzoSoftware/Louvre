#include "LLog.h"
#include "LPointer.h"
#include <LSeat.h>
#include <LCompositor.h>
#include <LWayland.h>
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
void LPointer::pointerMoveEvent(float dx, float dy)
{
    /* For simplification, we handle the event in pointerPosChangeEvent().
     * The input backends emit pointerMoveEvent() or pointerPosChangeEvent() but
     * not both.*/
    pointerPosChangeEvent(cursor()->posC().x() + dx * 1.4f,
                          cursor()->posC().y() + dy * 1.4f);
}
//! [pointerMoveEvent]

//! [pointerPosChangeEvent]
void LPointer::pointerPosChangeEvent(float x, float y)
{
    cursor()->setPosC(LPointF(x,y));

    // Repaint cursor outputs if hardware composition is not supported
    for(LOutput *output : cursor()->intersectedOutputs())
    {
        if(!cursor()->hasHardwareSupport(output))
            output->repaint();
    }

    // Update the drag & drop icon (if there was one)
    if(seat()->dndManager()->icon())
        cursor()->repaintOutputs();

    // Update the Toplevel size (if there was one being resized)
    if(resizingToplevel())
    {
        updateResizingToplevelSize();
        return;
    }

    // Update the Toplevel pos (if there was one being moved interactively)
    if(movingToplevel())
    {
        updateMovingToplevelPos();

        movingToplevel()->surface()->repaintOutputs();

        if(movingToplevel()->maximized())
            movingToplevel()->configureC(movingToplevel()->states() &~ LToplevelRole::Maximized);

        return;
    }

    // DO NOT GET CONFUSED! If we are in a drag & drop session, we call setDragginSurface(NULL) in case there is a surface being dragged.
    if(seat()->dndManager()->dragging())
        setDragginSurface(nullptr);

    // If there was a surface holding the left pointer button
    if(draggingSurface())
    {
        sendMoveEventC();
        return;
    }

    // Find the first surface under the cursor
    LSurface *surface = surfaceAtC(cursor()->posC());

    if(!surface)
    {
        setFocusC(nullptr);
        cursor()->useDefault();
        cursor()->setVisible(true);
    }
    else
    {
        if(focusSurface() == surface)
            sendMoveEventC();
        else
        {
            cursor()->useDefault();
            cursor()->setVisible(true);
            setFocusC(surface);
        }
    }
}
//! [pointerPosChangeEvent]

//! [pointerButtonEvent]
void LPointer::pointerButtonEvent(Button button, ButtonState state)
{
    if(state == Released && button == Left)
    {
        seat()->dndManager()->drop();
    }

    if(!focusSurface())
    {
        LSurface *surface = surfaceAtC(cursor()->posC());

        if(surface)
        {

            seat()->keyboard()->setFocus(surface);
            setFocusC(surface);

            sendButtonEvent(button,state);

            if(surface->popup())
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

    sendButtonEvent(button,state);

    if(button != Left)
        return;

    // Left button pressed
    if(state == Pressed)
    {
        /* We save the pointer focus surface in order to continue sending events to it even when the cursor 
         * is outside of it (while the left button is being held down)*/
        setDragginSurface(focusSurface());

        seat()->keyboard()->setFocus(focusSurface());

        if(focusSurface()->toplevel() && !focusSurface()->toplevel()->activated())
            focusSurface()->toplevel()->configureC(0, focusSurface()->toplevel()->states() | LToplevelRole::Activated);

        // Raise surface
        if(focusSurface()->parent())
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

        if(!focusSurface()->inputRegionC().containsPoint(focusSurface()->rolePosC() - cursor()->posC()))
        {
            setFocusC(nullptr);

            cursor()->useDefault();
            cursor()->setVisible(true);
        }

    }
}
//! [pointerButtonEvent]

#if LOUVRE_SEAT_VERSION >= 5

//! [pointerAxisEvent]
void LPointer::pointerAxisEvent(double x, double y, UInt32 source)
{
    // Inverts the scroll direction
    sendAxisEvent(-x,-y,source);
}
//! [pointerAxisEvent]

#else

//! [pointerAxisEvent_less_5]
void LPointer::pointerAxisEvent(double x, double y)
{
    // Inverts the scroll direction
    sendAxisEvent(-x,-y);
}
//! [pointerAxisEvent_less_5]

#endif

//! [setCursorRequest]
void LPointer::setCursorRequest(LCursorRole *cursorRole)
{
    if(cursorRole)
    {
        cursor()->setTextureB(
                    cursorRole->surface()->texture(),
                    cursorRole->hotspotB());

        cursor()->setVisible(true);

        // We notify the outputs that the cursor intersects for the current surface to update its scale
        for(LOutput *o : compositor()->outputs())
        {
            if(o == cursor()->output())
                cursorRole->surface()->sendOutputEnterEvent(o);
            else
                cursorRole->surface()->sendOutputLeaveEvent(o);
        }
    }
    // If nullptr means the client wants to hide the cursor
    else
        cursor()->setVisible(false);

}
//! [setCursorRequest]
