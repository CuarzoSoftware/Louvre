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

#include "Global.h"
#include "Pointer.h"
#include "Compositor.h"

Pointer::Pointer(Params *params) : LPointer(params) {}

void Pointer::pointerMoveEvent(Float32 dx, Float32 dy)
{
    dx *= 0.5f;
    dy *= 0.5f;

    if (LTime::ms() - lastEventMS > 40)
        velocity = 0.f;

    lastEventMS = LTime::ms();

    if (velocity.x() < 5.f)
        velocity.setX(velocity.x() + fabs(dx) * acelerationFactor);

    if (velocity.y() < 5.f)
        velocity.setY(velocity.y() + fabs(dy) * acelerationFactor);

    pointerPosChangeEvent(cursor()->pos().x() + dx + (velocity.x() * dx) / (fabs(dx) + 0.000001),
                          cursor()->pos().y() + dy + (velocity.y() * dy) / (fabs(dy) + 0.000001));
}

void Pointer::pointerPosChangeEvent(Float32 x, Float32 y)
{
    LView *view = G::scene()->handlePointerPosChangeEvent(x, y);

    if (movingToplevel() || resizingToplevel())
    {
        for (LOutput *o : cursor()->intersectedOutputs())
            o->repaint();
    }

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
    {
        G::enableDocks(true);
        G::compositor()->updatePointerBeforePaint = true;
    }

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
