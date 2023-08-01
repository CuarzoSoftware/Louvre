#include <LScene.h>
#include <LCursor.h>
#include <LXCursor.h>
#include <LView.h>
#include <LCursorRole.h>

#include "Global.h"
#include "Pointer.h"
#include "Surface.h"

Pointer::Pointer(Params *params) : LPointer(params) {}

void Pointer::pointerMoveEvent(Float32 dx, Float32 dy)
{
    LView *view = G::scene()->handlePointerMoveEvent(dx, dy);

    if (restoreCursor && !view)
    {
        cursor()->useDefault();
        cursor()->setVisible(true);
    }
}

void Pointer::pointerPosChangeEvent(Float32 x, Float32 y)
{
    LView *view = G::scene()->handlePointerPosChangeEvent(x, y);

    if (restoreCursor && !view)
    {
        cursor()->useDefault();
        cursor()->setVisible(true);
    }
}

void Pointer::pointerButtonEvent(Button button, ButtonState state)
{
    G::scene()->handlePointerButtonEvent(button, state);
}

void Pointer::pointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source)
{
    G::scene()->handlePointerAxisEvent(-axisX, -axisY, -discreteX, -discreteY, source);
}

void Pointer::setCursorRequest(LCursorRole *cursorRole)
{
    if (resizingToplevel() || !restoreCursor)
        return;

    if (cursorRole)
    {
        cursor()->setTextureB(
                    cursorRole->surface()->texture(),
                    cursorRole->hotspotB());

        cursor()->setVisible(true);

        // We notify the outputs that the cursor intersects for the current surface to update its scale
        for (LOutput *o : compositor()->outputs())
        {
            if (o == cursor()->output())
                cursorRole->surface()->sendOutputEnterEvent(o);
            else
                cursorRole->surface()->sendOutputLeaveEvent(o);
        }
    }
    // If nullptr means the client wants to hide the cursor
    else
        cursor()->setVisible(false);
}
