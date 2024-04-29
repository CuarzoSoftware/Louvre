#ifndef LSCENEPRIVATE_H
#define LSCENEPRIVATE_H

#include <LPointerEnterEvent.h>
#include <LPointerHoldEndEvent.h>
#include <LPointerLeaveEvent.h>
#include <LPointerMoveEvent.h>
#include <LPointerPinchEndEvent.h>
#include <LPointerSwipeEndEvent.h>
#include <LTouchDownEvent.h>
#include <LSceneView.h>
#include <LScene.h>
#include <LBitset.h>
#include <LSeat.h>
#include <mutex>

using namespace Louvre;

LPRIVATE_CLASS(LScene)

    enum State : UInt32
    {
        PointerIsBlocked                    = static_cast<UInt32>(1) << 0,
        TouchIsBlocked                      = static_cast<UInt32>(1) << 1,
        ChildrenListChanged                 = static_cast<UInt32>(1) << 2,
        PointerFocusVectorChanged           = static_cast<UInt32>(1) << 3,
        KeyboardFocusVectorChanged          = static_cast<UInt32>(1) << 4,
        TouchPointsVectorChanged            = static_cast<UInt32>(1) << 5,
        HandlingPointerMoveEvent            = static_cast<UInt32>(1) << 6,
        HandlingPointerButtonEvent          = static_cast<UInt32>(1) << 7,
        HandlingPointerScrollEvent          = static_cast<UInt32>(1) << 8,
        HandlingPointerSwipeBeginEvent      = static_cast<UInt32>(1) << 9,
        HandlingPointerSwipeUpdateEvent     = static_cast<UInt32>(1) << 10,
        HandlingPointerSwipeEndEvent        = static_cast<UInt32>(1) << 11,
        HandlingPointerPinchBeginEvent      = static_cast<UInt32>(1) << 12,
        HandlingPointerPinchUpdateEvent     = static_cast<UInt32>(1) << 13,
        HandlingPointerPinchEndEvent        = static_cast<UInt32>(1) << 14,
        HandlingPointerHoldBeginEvent       = static_cast<UInt32>(1) << 15,
        HandlingPointerHoldEndEvent         = static_cast<UInt32>(1) << 16,
        HandlingKeyboardKeyEvent            = static_cast<UInt32>(1) << 17,
        HandlingTouchEvent                  = static_cast<UInt32>(1) << 18,
    };

    LBitset<State> state;
    std::mutex mutex;
    LSceneView view;

    std::vector<LView*> pointerFocus;
    std::vector<LView*> keyboardFocus;
    std::vector<LSceneTouchPoint*> touchPoints;

    LPointF pointerMoveEventOutLocalPos;
    LPointerEnterEvent currentPointerEnterEvent;
    LPointerMoveEvent currentPointerMoveEvent;
    LPointerLeaveEvent currentPointerLeaveEvent;

    LPointerSwipeEndEvent pointerSwipeEndEvent;
    LPointerPinchEndEvent pointerPinchEndEvent;
    LPointerHoldEndEvent pointerHoldEndEvent;

    LTouchDownEvent touchDownEvent;
    LPointF touchGlobalPos;
    LSceneTouchPoint *currentTouchPoint;

    bool pointClippedByParent(LView *parent, const LPoint &point);
    bool pointClippedByParentScene(LView *view, const LPoint &point);
    LView *viewAt(LView *view, const LPoint &pos, LView::Type type, LBitset<LScene::InputFilter> flags);
    LPoint viewLocalPos(LView *view, const LPoint &pos);
    bool handlePointerMove(LView *view);
    bool handleTouchDown(LView *view);

    bool pointIsOverView(LView *view, const LPointF &pos, LBitset<LScene::InputFilter> flags)
    {
        if (!view->mapped() || (flags.check(InputFilter::Pointer) && !view->pointerEventsEnabled()) || (flags.check(InputFilter::Touch) && !view->touchEventsEnabled()))
            return false;

        if (view->clippingEnabled() && !view->clippingRect().containsPoint(pos))
            return false;

        if (pointClippedByParent(view, pos))
            return false;

        if (pointClippedByParentScene(view, pos))
            return false;

        if ((view->scalingEnabled() || view->parentScalingEnabled()) && view->scalingVector() != LSizeF(1.f,1.f))
        {
            if (view->scalingVector().area() == 0.f)
                return false;

            if (view->inputRegion())
            {
                if (view->inputRegion()->containsPoint((pos - view->pos())/view->scalingVector()))
                    return true;
            }
            else
            {
                if (LRect(view->pos(), view->size()).containsPoint((pos - view->pos())/view->scalingVector()))
                    return true;
            }
        }
        else
        {
            if (view->inputRegion())
            {
                if (view->inputRegion()->containsPoint(pos - view->pos()))
                    return true;
            }
            else
            {
                if (LRect(view->pos(), view->size()).containsPoint(pos))
                    return true;
            }
        }

        return false;
    }
};

#endif // LSCENEPRIVATE_H
