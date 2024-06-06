#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
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
#include "src/Client.h"

Pointer::Pointer(const void *params) : LPointer(params)
{
    enableNaturalScrollingX(false);
    enableNaturalScrollingY(false);
}

void Pointer::pointerMoveEvent(const LPointerMoveEvent &event)
{
    G::scene()->handlePointerMoveEvent(event);

    if (!seat()->toplevelMoveSessions().empty())
        cursor()->repaintOutputs(false);

    if (cursorOwner)
        return;

    if (seat()->dnd()->dragging())
    {
        cursor()->setCursor(seat()->dnd()->origin()->client()->lastCursorRequest());
        return;
    }

    for (auto *moveSession : seat()->toplevelMoveSessions())
    {
        if (moveSession->triggeringEvent().type() != LEvent::Type::Touch)
        {
            if (moveSession->toplevel()->decorationMode() == LToplevelRole::ClientSide)
                cursor()->setCursor(moveSession->toplevel()->client()->lastCursorRequest());
            return;
        }
    }

    for (auto *resizeSession : seat()->toplevelResizeSessions())
    {
        if (resizeSession->triggeringEvent().type() != LEvent::Type::Touch)
        {
            if (resizeSession->toplevel()->decorationMode() == LToplevelRole::ClientSide)
                cursor()->setCursor(resizeSession->toplevel()->client()->lastCursorRequest());
            return;
        }
    }

    if (G::scene()->pointerFocus().empty())
        return;

    if (G::scene()->pointerFocus().front()->userData() == WallpaperType)
    {
        cursor()->setVisible(true);
        cursor()->useDefault();
        return;
    }

    if (G::scene()->pointerFocus().front()->type() == LView::SurfaceType)
        cursor()->setCursor(static_cast<LSurfaceView*>(G::scene()->pointerFocus().back())->surface()->client()->lastCursorRequest());
}

void Pointer::pointerButtonEvent(const LPointerButtonEvent &event)
{
    if (event.button() == LPointerButtonEvent::Left && event.state() == LPointerButtonEvent::Released)
    {
        G::enableDocks(true);
        G::compositor()->updatePointerBeforePaint = true;
    }

    G::scene()->handlePointerButtonEvent(event);

    if (G::compositor()->wofiClient && !G::scene()->pointerFocus().empty() && G::scene()->pointerFocus().front()->userData() == WallpaperType)
        G::compositor()->wofiClient->destroyLater();
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
