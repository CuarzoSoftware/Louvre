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

bool Pointer::maybeMoveOrResize(const LPointerButtonEvent &event) {
    if (event.state() != LPointerButtonEvent::Pressed) return false;
    if (!seat()->keyboard()->isKeyCodePressed(KEY_LEFTMETA)) return false;

    LPointF mpos { cursor()->pos() };

    // XXX: I have the feeling there's a simpler way to find the Toplevel
    // object, but this works.
    LView *view { G::scene()->viewAt(mpos, LView::UndefinedType, LScene::InputFilter::Pointer) };
    Toplevel *toplevel { nullptr };
    if (view && view->type() == LView::SurfaceType) {
        Surface *surface = dynamic_cast<Surface*>( static_cast<LSurfaceView*>(view)->surface() );
        while (surface) {
            if ((toplevel = surface->tl())) break;
            surface = (Surface*)surface->parent();
            if (surface && surface->views().size() > 0)
                view = surface->views()[0];
        }
    }
    if (!toplevel) return false;

    // Meta + Left Click to move window
    if (event.button() == LPointerButtonEvent::Left) {
        toplevel->startMoveRequest(event);
        return true;
    }

    // Meta + Right Click to resize window
    if (event.button() == LPointerButtonEvent::Right) {
        LPointF vpos { view->pos() };
        LSizeF vsz { view->size() };
        LBitset<LEdge> anchor;

        if (mpos.x() >= vpos.x() + 0.0000f * vsz.x() && mpos.x() < vpos.x() + 0.3333f * vsz.x() &&
            mpos.y() >= vpos.y() + 0.0000f * vsz.y() && mpos.y() < vpos.y() + 0.3333f * vsz.y())
            anchor = LEdgeTop | LEdgeLeft;

        if (mpos.x() >= vpos.x() + 0.3333f * vsz.x() && mpos.x() < vpos.x() + 0.6666f * vsz.x() &&
            mpos.y() >= vpos.y() + 0.0000f * vsz.y() && mpos.y() < vpos.y() + 0.3333f * vsz.y())
            anchor = LEdgeTop;

        if (mpos.x() >= vpos.x() + 0.6666f * vsz.x() && mpos.x() < vpos.x() + 1 * vsz.x() &&
            mpos.y() >= vpos.y() + 0.0000f * vsz.y() && mpos.y() < vpos.y() + 0.3333f * vsz.y())
            anchor = LEdgeTop | LEdgeRight;

        if (mpos.x() >= vpos.x() + 0.0000f * vsz.x() && mpos.x() < vpos.x() + 0.3333f * vsz.x() &&
            mpos.y() >= vpos.y() + 0.3333f * vsz.y() && mpos.y() < vpos.y() + 0.6666f * vsz.y())
            anchor = LEdgeLeft;

        if (mpos.x() >= vpos.x() + 0.3333f * vsz.x() && mpos.x() < vpos.x() + 0.6666f * vsz.x() &&
            mpos.y() >= vpos.y() + 0.3333f * vsz.y() && mpos.y() < vpos.y() + 0.6666f * vsz.y())
            anchor = 0;

        if (mpos.x() >= vpos.x() + 0.6666f * vsz.x() && mpos.x() < vpos.x() + 1 * vsz.x() &&
            mpos.y() >= vpos.y() + 0.3333f * vsz.y() && mpos.y() < vpos.y() + 0.6666f * vsz.y())
            anchor = LEdgeRight;

        if (mpos.x() >= vpos.x() + 0.0000f * vsz.x() && mpos.x() < vpos.x() + 0.3333f * vsz.x() &&
            mpos.y() >= vpos.y() + 0.6666f * vsz.y() && mpos.y() < vpos.y() + 1 * vsz.y())
            anchor = LEdgeBottom | LEdgeLeft;

        if (mpos.x() >= vpos.x() + 0.3333f * vsz.x() && mpos.x() < vpos.x() + 0.6666f * vsz.x() &&
            mpos.y() >= vpos.y() + 0.6666f * vsz.y() && mpos.y() < vpos.y() + 1 * vsz.y())
            anchor = LEdgeBottom;

        if (mpos.x() >= vpos.x() + 0.6666f * vsz.x() && mpos.x() < vpos.x() + 1 * vsz.x() &&
            mpos.y() >= vpos.y() + 0.6666f * vsz.y() && mpos.y() < vpos.y() + 1 * vsz.y())
            anchor = LEdgeBottom | LEdgeRight;

        if (anchor) {
            toplevel->startResizeRequest(LPointerButtonEvent(), anchor);
        } else {
            toplevel->startMoveRequest(event);
        }
        return true;
    }

    return false;
}

void Pointer::pointerButtonEvent(const LPointerButtonEvent &event)
{
    if (maybeMoveOrResize(event)) {
        return;
    }

    if (event.button() == LPointerButtonEvent::Left && event.state() == LPointerButtonEvent::Released)
    {
        G::enableDocks(true);
        G::compositor()->updatePointerBeforePaint = true;
    }

    G::scene()->handlePointerButtonEvent(event);

    if (G::compositor()->wofi && G::compositor()->wofi->client && G::compositor()->wofi && !G::scene()->pointerFocus().empty() && G::scene()->pointerFocus().front()->userData() == WallpaperType)
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
