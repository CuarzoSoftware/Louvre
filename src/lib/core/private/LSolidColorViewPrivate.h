#ifndef LSOLIDCOLORVIEWPRIVATE_H
#define LSOLIDCOLORVIEWPRIVATE_H

#include <LSolidColorView.h>
#include <LRegion.h>

using namespace Louvre;

LPRIVATE_CLASS(LSolidColorView)
    LRGBF color;
    LPoint nativePos;
    LSize nativeSize;
    LRegion *inputRegion = nullptr;
    LRegion opaqueRegion;
    LRegion emptyRegion;
    std::vector<LOutput *>outputs;

    LPoint tmpPos;
};

#endif // LSOLIDCOLORVIEWPRIVATE_H
