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
        cursor()->output()->repaint();

    if (resizingToplevel() || cursorOwner)
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
