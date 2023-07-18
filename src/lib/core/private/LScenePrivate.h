#ifndef LSCENEPRIVATE_H
#define LSCENEPRIVATE_H

#include <LScene.h>

using namespace Louvre;

LPRIVATE_CLASS(LScene)
    LSceneView *view;
    bool pointClippedByParent(LView *parent, const LPoint &point);
    LView *viewAtC(LView *view, const LPoint &pos);
};

#endif // LSCENEPRIVATE_H
