#ifndef LSURFACEVIEWPRIVATE_H
#define LSURFACEVIEWPRIVATE_H

#include <LSurfaceView.h>
#include <LPoint.h>

using namespace Louvre;

LPRIVATE_CLASS(LSurfaceView)
    LSurface *surface;
    LRegion *customInputRegion = nullptr;
    LPoint customPos;

    bool primary = true;
    bool customPosEnabled = false;
    bool customInputRegionEnabled = false;

    LPoint tmpPos;
};

#endif // LSURFACEVIEWPRIVATE_H
