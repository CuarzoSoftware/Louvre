#include <LScene.h>
#include <LCursor.h>
#include <LXCursor.h>
#include <LView.h>
#include <LCursorRole.h>
#include <LLog.h>
#include <LTime.h>
#include <LSurfaceView.h>
#include <LSurface.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LDNDManager.h>

#include "Global.h"
#include "Pointer.h"
#include "Compositor.h"

Pointer::Pointer(const void *params) : LPointer(params) {}

void Pointer::pointerMoveEvent(Float32 x, Float32 y, bool absolute)
{
    LView *view = G::scene()->handlePointerMoveEvent(x, y, absolute);

    if (movingToplevel() || resizingToplevel())
        cursor()->output()->repaint();

    if (resizingToplevel() || cursorOwner)
        return;

    // Let the client set the cursor during DND
    if (seat()->dndManager()->dragging())
        return;

    if (view)
    {
        if (view->type() == LView::Surface)
        {
            LSurfaceView *surfView = (LSurfaceView*)view;

            if (lastCursorRequest() && lastCursorRequest()->surface()->client() == surfView->surface()->client())
            {
                cursor()->setTextureB(lastCursorRequest()->surface()->texture(), lastCursorRequest()->hotspotB());
                cursor()->setVisible(true);
            }
            else
            {
                if (lastCursorRequestWasHide())
                    cursor()->setVisible(false);
                else
                {
                    cursor()->setVisible(true);
                    cursor()->useDefault();
                }
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
    {
        G::enableDocks(true);
        G::compositor()->updatePointerBeforePaint = true;
    }

    G::scene()->handlePointerButtonEvent(button, state);
}

void Pointer::pointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, AxisSource source)
{
    Float32 speed = 0.6f;
    G::scene()->handlePointerAxisEvent(-axisX * speed,
                                       -axisY * speed,
                                       -discreteX * speed,
                                       -discreteY * speed,
                                       source);
}

void Pointer::setCursorRequest(LCursorRole *cursorRole)
{
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
