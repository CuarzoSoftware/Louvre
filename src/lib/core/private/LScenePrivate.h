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
        PointerIsBlocked                    = 1 << 0,
        TouchIsBlocked                      = 1 << 1,
        ChildrenListChanged                 = 1 << 2,
        PointerFocusVectorChanged           = 1 << 3,
        KeyboardFocusVectorChanged          = 1 << 4,
        TouchPointsVectorChanged            = 1 << 5,
        HandlingPointerMoveEvent            = 1 << 6,
        HandlingPointerButtonEvent          = 1 << 7,
        HandlingPointerScrollEvent          = 1 << 8,
        HandlingPointerSwipeBeginEvent      = 1 << 9,
        HandlingPointerSwipeUpdateEvent     = 1 << 10,
        HandlingPointerSwipeEndEvent        = 1 << 11,
        HandlingPointerPinchBeginEvent      = 1 << 12,
        HandlingPointerPinchUpdateEvent     = 1 << 13,
        HandlingPointerPinchEndEvent        = 1 << 14,
        HandlingPointerHoldBeginEvent       = 1 << 15,
        HandlingPointerHoldEndEvent         = 1 << 16,
        HandlingKeyboardKeyEvent            = 1 << 17,
        HandlingTouchEvent                  = 1 << 18,
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
    LView *viewAt(LView *view, const LPoint &pos, LView::Type type, LSeat::InputCapabilitiesFlags flags);
    LPoint viewLocalPos(LView *view, const LPoint &pos);
    bool handlePointerMove(LView *view);
    bool handleTouchDown(LView *view);

    inline bool pointIsOverView(LView *view, const LPointF &pos, LSeat::InputCapabilitiesFlags flags)
    {
        if (!view->mapped() || (flags & LSeat::Pointer && !view->pointerEventsEnabled()) || (flags & LSeat::Touch && !view->touchEventsEnabled()))
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
