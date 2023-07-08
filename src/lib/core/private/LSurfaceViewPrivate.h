#ifndef LSURFACEVIEWPRIVATE_H
#define LSURFACEVIEWPRIVATE_H

#include <LSurfaceView.h>
#include <LPoint.h>

using namespace Louvre;

LPRIVATE_CLASS(LSurfaceView)
    LSurface *surface;
    bool primary;
    bool customPosEnabled;
    bool forceRequestNextFrameEnabled = false;
    LPoint customPosC;
    LPoint tmpPosC;
};

#endif // LSURFACEVIEWPRIVATE_H
