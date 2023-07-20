#ifndef LSCENEPRIVATE_H
#define LSCENEPRIVATE_H

#include <LScene.h>

using namespace Louvre;

LPRIVATE_CLASS(LScene)
    LSceneView *view;
    bool pointClippedByParent(LView *parent, const LPoint &point);
    bool pointClippedByParentScene(LView *view, const LPoint &point);
    LView *viewAt(LView *view, const LPoint &pos);
};

#endif // LSCENEPRIVATE_H
