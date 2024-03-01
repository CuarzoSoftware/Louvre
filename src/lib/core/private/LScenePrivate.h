#ifndef LSCENEPRIVATE_H
#define LSCENEPRIVATE_H

#include <LSceneView.h>
#include <LScene.h>
#include <mutex>

using namespace Louvre;

LPRIVATE_CLASS(LScene)
    std::mutex mutex;
    LSceneView view;
    bool handleWaylandPointerEvents = true;
    bool handleWaylandKeyboardEvents = true;
    bool auxKeyboardImplementationEnabled = true;
    bool pointerIsBlocked = false;

    bool listChanged = false;

    // Prevent recursive calls
    bool handlingPointerMove = false;
    bool handlingPointerButton = false;
    bool handlingPointerAxisEvent = false;
    bool handlingKeyModifiersEvent = false;
    bool handlingKeyEvent = false;

    bool pointClippedByParent(LView *parent, const LPoint &point);
    bool pointClippedByParentScene(LView *view, const LPoint &point);
    LView *viewAt(LView *view, const LPoint &pos);
    LPoint viewLocalPos(LView *view, const LPoint &pos);
    bool pointerIsOverView(LView *view, const LPoint &pos);
    bool handlePointerMove(LView *view, const LPoint &pos, LView **firstViewFound);
    bool handlePointerButton(LView *view, LPointerButtonEvent::Button button, LPointerButtonEvent::State state);
    bool handlePointerAxisEvent(LView *view, Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source);
    bool handleKeyModifiersEvent(LView *view, UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group);
    bool handleKeyEvent(LView *view, UInt32 keyCode, UInt32 keyState);
};

#endif // LSCENEPRIVATE_H
