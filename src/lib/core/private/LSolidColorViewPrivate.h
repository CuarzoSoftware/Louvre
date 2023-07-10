#ifndef LSOLIDCOLORVIEWPRIVATE_H
#define LSOLIDCOLORVIEWPRIVATE_H

#include <LSolidColorView.h>
#include <LRegion.h>

using namespace Louvre;

LPRIVATE_CLASS(LSolidColorView)
    LRGBF color;
    LPoint posC;
    LPoint tmpPos;
    LRegion inputRegionC;
    LRegion opaqueRegionC;
    LRegion translucentRegionC;
    LRegion damageC;
    std::list<LOutput*>outputs;
};

#endif // LSOLIDCOLORVIEWPRIVATE_H
