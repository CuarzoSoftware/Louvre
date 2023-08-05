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

    if (resizingToplevel() || cursorOwner)
        return;

    if (view)
    {
        if (view->type() == LView::Surface)
        {
            LSurfaceView *surfView = (LSurfaceView*)view;

            if (surfView->surface() == lastCursorRequestFocusedSurface)
            {
                if (lastCursorRequest())
                {
                    cursor()->setTextureB(lastCursorRequest()->surface()->texture(), lastCursorRequest()->hotspotB());
                    cursor()->setVisible(true);
                }
                else
                    cursor()->setVisible(false);
            }
        }

        return;
    }

    cursor()->useDefault();
    cursor()->setVisible(true);
}

void Pointer::pointerPosChangeEvent(Float32 x, Float32 y)
{
    LView *view = G::scene()->handlePointerPosChangeEvent(x, y);

    if (resizingToplevel() || cursorOwner)
        return;

    if (view)
    {
        if (view->type() == LView::Surface)
        {
            LSurfaceView *surfView = (LSurfaceView*)view;

            if (surfView->surface() == lastCursorRequestFocusedSurface)
            {
                if (lastCursorRequest())
                {
                    cursor()->setTextureB(lastCursorRequest()->surface()->texture(), lastCursorRequest()->hotspotB());
                    cursor()->setVisible(true);
                }
                else
                    cursor()->setVisible(false);
            }
        }

        return;
    }

    cursor()->useDefault();
    cursor()->setVisible(true);
}

void Pointer::pointerButtonEvent(Button button, ButtonState state)
{
    if (button == LPointer::Left && state == LPointer::Released)
        G::compositor()->updatePointerBeforePaint = true;

    G::scene()->handlePointerButtonEvent(button, state);
}

void Pointer::pointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source)
{
    G::scene()->handlePointerAxisEvent(-axisX, -axisY, -discreteX, -discreteY, source);
}

void Pointer::setCursorRequest(LCursorRole *cursorRole)
{
    lastCursorRequestFocusedSurface = focusSurface();

    if (resizingToplevel() || cursorOwner)
        return;

    if (cursorRole)
    {
        cursor()->setTextureB(
                    cursorRole->surface()->texture(),
                    cursorRole->hotspotB());

        cursor()->setVisible(true);
    }
    // If nullptr means the client wants to hide the cursor
    else
        cursor()->setVisible(false);
}
