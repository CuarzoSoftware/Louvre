#ifndef LSCENEPRIVATE_H
#define LSCENEPRIVATE_H

#include <LScene.h>

using namespace Louvre;

LPRIVATE_CLASS(LScene)
    LSceneView *view;
    bool handleWaylandPointerEvents = true;
    bool pointClippedByParent(LView *parent, const LPoint &point);
    bool pointClippedByParentScene(LView *view, const LPoint &point);
    LView *viewAt(LView *view, const LPoint &pos);
    LPoint viewLocalPos(LView *view, const LPoint &pos);
    bool pointerIsOverView(LView *view, const LPoint &pos);
    void handlePointerMove(LView *view, const LPoint &pos, LView **firstViewFound);
    void handlePointerButton(LView *view, LPointer::Button button, LPointer::ButtonState state);
    void handlePointerAxisEvent(LView *view, Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source);
};

#endif // LSCENEPRIVATE_H
