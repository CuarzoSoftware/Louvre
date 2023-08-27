#ifndef LSCENEPRIVATE_H
#define LSCENEPRIVATE_H

#include <LScene.h>
#include <mutex>

using namespace Louvre;

LPRIVATE_CLASS(LScene)
    std::mutex mutex;
    LSceneView *view;
    bool handleWaylandPointerEvents = true;
    bool handleWaylandKeyboardEvents = true;
    bool auxKeyboardImplementationEnabled = true;
    bool pointerIsBlocked = false;
    bool pointClippedByParent(LView *parent, const LPoint &point);
    bool pointClippedByParentScene(LView *view, const LPoint &point);
    LView *viewAt(LView *view, const LPoint &pos);
    LPoint viewLocalPos(LView *view, const LPoint &pos);
    bool pointerIsOverView(LView *view, const LPoint &pos);
    void handlePointerMove(LView *view, const LPoint &pos, LView **firstViewFound);
    void handlePointerButton(LView *view, LPointer::Button button, LPointer::ButtonState state);
    void handlePointerAxisEvent(LView *view, Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source);

    void handleKeyModifiersEvent(LView *view, UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group);
    void handleKeyEvent(LView *view, UInt32 keyCode, UInt32 keyState);
};

#endif // LSCENEPRIVATE_H
