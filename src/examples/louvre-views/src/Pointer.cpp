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

#include "App.h"
#include "Global.h"
#include "Pointer.h"
#include "Compositor.h"
#include "Surface.h"
#include "Toplevel.h"
#include "ToplevelView.h"
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

bool Pointer::maybeMoveOrResize(const LPointerButtonEvent &event)
{
    if (!focus()
        || event.state() != LPointerButtonEvent::Pressed
        || !seat()->keyboard()->isKeyCodePressed(KEY_LEFTMETA)
        || !(event.button() == LPointerButtonEvent::Left || event.button() == LPointerButtonEvent::Right))
        return false;

    Toplevel *toplevel { focus()->toplevel() ?
        static_cast<Toplevel*>(focus()->toplevel()) :
        static_cast<Surface*>(focus())->closestToplevelParent() };

    if (!toplevel || toplevel->fullscreen() || toplevel->resizeSession().isActive() || toplevel->moveSession().isActive())
        return false;

    const LPointF mpos { cursor()->pos() };
    LView *view { toplevel->surf()->getView() };
    LBitset<LEdge> anchor { 0 };
    LXCursor *cursor { G::cursors().move };

    // Meta + Right Click to resize window
    if (event.button() == LPointerButtonEvent::Right)
    {
        LPointF vpos { view->pos() };
        LSizeF vsz { view->size() };

        if (mpos.x() >= vpos.x() + 0.0000f * vsz.x() && mpos.x() < vpos.x() + 0.3333f * vsz.x() &&
            mpos.y() >= vpos.y() + 0.0000f * vsz.y() && mpos.y() < vpos.y() + 0.3333f * vsz.y())
        {
            anchor = LEdgeTop | LEdgeLeft;
            cursor = G::cursors().top_left_corner;
        }
        else if (mpos.x() >= vpos.x() + 0.3333f * vsz.x() && mpos.x() < vpos.x() + 0.6666f * vsz.x() &&
                 mpos.y() >= vpos.y() + 0.0000f * vsz.y() && mpos.y() < vpos.y() + 0.3333f * vsz.y())
        {
            anchor = LEdgeTop;
            cursor = G::cursors().top_side;
        }
        else if (mpos.x() >= vpos.x() + 0.6666f * vsz.x() && mpos.x() < vpos.x() + 1 * vsz.x() &&
                 mpos.y() >= vpos.y() + 0.0000f * vsz.y() && mpos.y() < vpos.y() + 0.3333f * vsz.y())
        {
            anchor = LEdgeTop | LEdgeRight;
            cursor = G::cursors().top_right_corner;
        }
        else if (mpos.x() >= vpos.x() + 0.0000f * vsz.x() && mpos.x() < vpos.x() + 0.3333f * vsz.x() &&
                 mpos.y() >= vpos.y() + 0.3333f * vsz.y() && mpos.y() < vpos.y() + 0.6666f * vsz.y())
        {
            anchor = LEdgeLeft;
            cursor = G::cursors().left_side;
        }
        else if (mpos.x() >= vpos.x() + 0.6666f * vsz.x() && mpos.x() < vpos.x() + 1 * vsz.x() &&
                 mpos.y() >= vpos.y() + 0.3333f * vsz.y() && mpos.y() < vpos.y() + 0.6666f * vsz.y())
        {
            anchor = LEdgeRight;
            cursor = G::cursors().right_side;
        }
        else if (mpos.x() >= vpos.x() + 0.0000f * vsz.x() && mpos.x() < vpos.x() + 0.3333f * vsz.x() &&
                 mpos.y() >= vpos.y() + 0.6666f * vsz.y() && mpos.y() < vpos.y() + 1 * vsz.y())
        {
            anchor = LEdgeBottom | LEdgeLeft;
            cursor = G::cursors().bottom_left_corner;
        }
        else if (mpos.x() >= vpos.x() + 0.3333f * vsz.x() && mpos.x() < vpos.x() + 0.6666f * vsz.x() &&
                 mpos.y() >= vpos.y() + 0.6666f * vsz.y() && mpos.y() < vpos.y() + 1 * vsz.y())
        {
            anchor = LEdgeBottom;
            cursor = G::cursors().bottom_side;
        }
        else if (mpos.x() >= vpos.x() + 0.6666f * vsz.x() && mpos.x() < vpos.x() + 1 * vsz.x() &&
                 mpos.y() >= vpos.y() + 0.6666f * vsz.y() && mpos.y() < vpos.y() + 1 * vsz.y())
        {
            anchor = LEdgeBottom | LEdgeRight;
            cursor = G::cursors().bottom_right_corner;
        }
    }

    Louvre::cursor()->setCursor(cursor);
    cursorOwner = view;

    if (anchor)
        toplevel->startResizeRequest(event, anchor);
    else
        toplevel->startMoveRequest(event);

    return true;
}

void Pointer::pointerButtonEvent(const LPointerButtonEvent &event)
{
    if (maybeMoveOrResize(event))
        G::scene()->handlePointerButtonEvent(event, 0);
    else
        G::scene()->handlePointerButtonEvent(event);

    if (event.state() == LPointerButtonEvent::Released
        && seat()->toplevelResizeSessions().empty()
        && seat()->toplevelMoveSessions().empty())
    {
        G::compositor()->cursor()->useDefault();
        G::enableDocks(true);
        G::compositor()->updatePointerBeforePaint = true;
    }

    if (G::compositor()->wofi && G::compositor()->wofi->client
        && G::compositor()->wofi && !G::scene()->pointerFocus().empty()
        && G::scene()->pointerFocus().front()->userData() == WallpaperType)
        G::compositor()->wofi->client->destroyLater();
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
