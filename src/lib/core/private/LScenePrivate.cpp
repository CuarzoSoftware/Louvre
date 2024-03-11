#include <private/LScenePrivate.h>
#include <private/LSceneViewPrivate.h>
#include <private/LSceneTouchPointPrivate.h>
#include <LOutput.h>
#include <LCompositor.h>
#include <LPainter.h>
#include <LSurfaceView.h>
#include <LFramebuffer.h>
#include <LCursor.h>
#include <LLog.h>

using LVS = LView::LViewState;
using LSS = LScene::LScenePrivate::State;

LView *LScene::LScenePrivate::viewAt(LView *view, const LPoint &pos, LView::Type type, LSeat::InputCapabilitiesFlags flags)
{
    LView *v { nullptr };

    for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
    {
        v = viewAt(*it, pos, type, flags);

        if (v)
            return v;
    }

    if (!view->mapped())
        return nullptr;

    if (type != LView::Undefined && view->type() != type)
        return nullptr;

    if ((flags & LSeat::Touch && !view->touchEventsEnabled()) && (flags & LSeat::Pointer && !view->pointerEventsEnabled()))
        return nullptr;

    if (view->clippingEnabled() && !view->clippingRect().containsPoint(pos))
        return nullptr;

    if (pointClippedByParent(view, pos))
        return nullptr;

    if (pointClippedByParentScene(view, pos))
        return nullptr;

    if (flags == 0)
        return view;

    if ((view->scalingEnabled() || view->parentScalingEnabled()) && view->scalingVector() != LSizeF(1.f, 1.f))
    {
        if (view->scalingVector().area() == 0.f)
            return nullptr;

        if (view->inputRegion())
        {
            if (view->inputRegion()->containsPoint((pos - view->pos())/view->scalingVector()))
                v = view;
        }
        else
        {
            if (LRect(view->pos(), view->size()).containsPoint((pos - view->pos())/view->scalingVector()))
                v = view;
        }
    }
    else
    {
        if (view->inputRegion())
        {
            if (view->inputRegion()->containsPoint(pos - view->pos()))
                v = view;
        }
        else
        {
            if (LRect(view->pos(), view->size()).containsPoint(pos))
                v = view;
        }
    }

    return v;
}

bool LScene::LScenePrivate::pointClippedByParent(LView *view, const LPoint &point)
{
    if (!view->parent())
        return false;

    if (view->parentClippingEnabled())
    {
        if (!LRect(view->parent()->pos(), view->parent()->size()).containsPoint(point))
            return true;
    }

    return pointClippedByParent(view->parent(), point);
}

bool LScene::LScenePrivate::pointClippedByParentScene(LView *view, const LPoint &point)
{
    LSceneView *parentScene = view->parentSceneView();

    if (!parentScene)
        return false;

    if (parentScene->isLScene())
        return false;

    if (!parentScene->imp()->fb->rect().containsPoint(point))
        return true;

    return pointClippedByParentScene(parentScene, point);
}

bool LScene::LScenePrivate::handlePointerMove(LView *view)
{
    if (state.check(LSS::ChildrenListChanged))
        goto listChangedErr;

    for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        if (!handlePointerMove(*it))
            return false;

    if (!state.check(LSS::PointerIsBlocked) && pointIsOverView(view, cursor()->pos(), LSeat::Pointer))
    {
        if (!view->m_state.check(LVS::PointerMoveDone))
        {
            view->m_state.add(LVS::PointerMoveDone);

            if (view->m_state.check(LVS::PointerIsOver))
            {
                LVectorRemoveOne(pointerFocus, view);
                pointerFocus.push_back(view);
                currentPointerMoveEvent.localPos = viewLocalPos(view, cursor()->pos());
                view->pointerMoveEvent(currentPointerMoveEvent);

                if (state.check(LSS::ChildrenListChanged))
                    goto listChangedErr;
            }
            else
            {
                view->m_state.add(LVS::PointerIsOver);
                pointerFocus.push_back(view);
                currentPointerEnterEvent.localPos = viewLocalPos(view, cursor()->pos());
                view->pointerEnterEvent(currentPointerEnterEvent);

                if (state.check(LSS::ChildrenListChanged))
                    goto listChangedErr;
            }
        }

        if (view->blockPointerEnabled())
            state.add(LSS::PointerIsBlocked);
    }
    else
    {
        if (!view->m_state.check(LVS::PointerMoveDone))
        {
            view->m_state.add(LVS::PointerMoveDone);

            if (view->m_state.check(LVS::PointerIsOver))
            {
                view->m_state.remove(LVS::PointerIsOver);

                if (view->m_state.check(LVS::PendingSwipeEnd))
                {
                    view->m_state.remove(LVS::PendingSwipeEnd);
                    pointerSwipeEndEvent.setCancelled(true);
                    pointerSwipeEndEvent.setMs(currentPointerMoveEvent.ms());
                    pointerSwipeEndEvent.setUs(currentPointerMoveEvent.us());
                    pointerSwipeEndEvent.setSerial(LTime::nextSerial());
                    view->pointerSwipeEndEvent(pointerSwipeEndEvent);
                }

                if (view->m_state.check(LVS::PendingPinchEnd))
                {
                    view->m_state.remove(LVS::PendingPinchEnd);
                    pointerPinchEndEvent.setCancelled(true);
                    pointerPinchEndEvent.setMs(currentPointerMoveEvent.ms());
                    pointerPinchEndEvent.setUs(currentPointerMoveEvent.us());
                    pointerPinchEndEvent.setSerial(LTime::nextSerial());
                    view->pointerPinchEndEvent(pointerPinchEndEvent);
                }

                if (view->m_state.check(LVS::PendingHoldEnd))
                {
                    view->m_state.remove(LVS::PendingHoldEnd);
                    pointerHoldEndEvent.setCancelled(true);
                    pointerHoldEndEvent.setMs(currentPointerMoveEvent.ms());
                    pointerHoldEndEvent.setUs(currentPointerMoveEvent.us());
                    pointerHoldEndEvent.setSerial(LTime::nextSerial());
                    view->pointerHoldEndEvent(pointerHoldEndEvent);
                }

                LVectorRemoveOne(pointerFocus, view);
                view->pointerLeaveEvent(currentPointerLeaveEvent);

                if (state.check(LSS::ChildrenListChanged))
                    goto listChangedErr;
            }
        }
    }

    return true;

    // If a list was modified, start again, serials are used to prevent resend events
listChangedErr:
    state.remove(LSS::ChildrenListChanged);
    handlePointerMove(&this->view);
    return false;
}

LPoint LScene::LScenePrivate::viewLocalPos(LView *view, const LPoint &pos)
{
    if ((view->scalingEnabled() || view->parentScalingEnabled()) && view->scalingVector().area() != 0.f)
        return (pos - view->pos()) / view->scalingVector();
    else
        return pos - view->pos();
}

bool LScene::LScenePrivate::handleTouchDown(LView *view)
{
    if (state.check(ChildrenListChanged))
        goto listChangedErr;

    for (std::list<LView*>::const_reverse_iterator it = view->children().crbegin(); it != view->children().crend(); it++)
        if (!handleTouchDown(*it))
            return false;

    if (!state.check(TouchIsBlocked) && pointIsOverView(view, touchGlobalPos, LSeat::Touch))
    {
        if (!view->m_state.check(LVS::TouchDownDone))
        {
            view->m_state.add(LVS::TouchDownDone);

            LVectorRemoveOne(currentTouchPoint->imp()->views, view);
            currentTouchPoint->imp()->views.push_back(view);
            touchDownEvent.localPos = viewLocalPos(view, touchGlobalPos);
            view->touchDownEvent(touchDownEvent);

            if (state.check(ChildrenListChanged))
                goto listChangedErr;
        }

        if (view->blockTouchEnabled())
            state.add(TouchIsBlocked);
    }

    return true;

// If a list was modified, start again, serials are used to prevent resend events
listChangedErr:
    state.remove(ChildrenListChanged);
    handleTouchDown(&this->view);
    return false;
}
