#ifndef LPAINTERMASKPRIVATE_H
#define LPAINTERMASKPRIVATE_H

#include <LPainterMask.h>
#include <LRect.h>

using namespace Louvre;

LPRIVATE_CLASS(LPainterMask)
    BlendMode blendMode;
    LRect rect;
    UInt32 type;
};

#endif // LPAINTERMASKPRIVATE_H
