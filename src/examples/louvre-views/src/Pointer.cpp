#include <LClientCursor.h>
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
#include <LClient.h>
#include <LDND.h>

#include "Global.h"
#include "Pointer.h"
#include "Compositor.h"

Pointer::Pointer(const void *params) : LPointer(params) {}

void Pointer::pointerMoveEvent(const LPointerMoveEvent &event)
{
    G::scene()->handlePointerMoveEvent(event);

    if (cursorOwner || !seat()->toplevelResizeSessions().empty())
        return;

    if (seat()->dnd()->dragging())
    {
        cursor()->setCursor(seat()->dnd()->origin()->client()->lastCursorRequest());
        return;
    }

    if (seat()->toplevelMoveSessions().empty())
    {
        if (G::scene()->pointerFocus().empty())
            return;

        if (G::scene()->pointerFocus().front()->id == WallpaperType)
        {
            cursor()->setVisible(true);
            cursor()->useDefault();
            return;
        }

        if (G::scene()->pointerFocus().front()->type() == LView::Surface)
            cursor()->setCursor(static_cast<LSurfaceView*>(G::scene()->pointerFocus().back())->surface()->client()->lastCursorRequest());
    }
}

void Pointer::pointerButtonEvent(const LPointerButtonEvent &event)
{
    if (event.button() == LPointerButtonEvent::Left && event.state() == LPointerButtonEvent::Released)
    {
        G::enableDocks(true);
        G::compositor()->updatePointerBeforePaint = true;
    }

    G::scene()->handlePointerButtonEvent(event);
}

void Pointer::pointerScrollEvent(const LPointerScrollEvent &event)
{
    G::scene()->handlePointerScrollEvent(event);
}

void Pointer::setCursorRequest(const LClientCursor &clientCursor)
{
    if (cursorOwner || !seat()->toplevelResizeSessions().empty())
        return;

    if (seat()->dnd()->dragging())
    {
        if (seat()->dnd()->origin()->client() == clientCursor.client())
            cursor()->setCursor(clientCursor);

        return;
    }

    if (focus() && focus()->client() == clientCursor.client())
        cursor()->setCursor(clientCursor);
}
