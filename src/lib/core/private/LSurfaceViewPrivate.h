#ifndef LSURFACEVIEWPRIVATE_H
#define LSURFACEVIEWPRIVATE_H

#include <LSurfaceView.h>
#include <LPoint.h>

using namespace Louvre;

LPRIVATE_CLASS(LSurfaceView)
    LSurface *surface;
    std::list<LSurfaceView*>::iterator surfaceLink;

    LRegion *customInputRegion = nullptr;
    LRegion *customTranslucentRegion = nullptr;
    LPoint customPos;

    std::list<LOutput*>nonPrimaryOutputs;

    bool primary = true;
    bool customPosEnabled = false;
    bool customInputRegionEnabled = false;
    bool customTranslucentRegionEnabled = false;

    LPoint tmpPos;
    LRectF customSrcRect;
};

#endif // LSURFACEVIEWPRIVATE_H
