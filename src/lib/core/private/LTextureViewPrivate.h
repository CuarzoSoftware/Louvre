#ifndef LTEXTUREVIEWPRIVATE_H
#define LTEXTUREVIEWPRIVATE_H

#include <LTextureView.h>
#include <LRegion.h>

using namespace Louvre;

LPRIVATE_CLASS(LTextureView)
    LTexture *texture = nullptr;
    LRegion *inputRegion = nullptr;
    LRegion *translucentRegion = nullptr;
    LRegion emptyRegion;
    LPoint nativePos;
    Int32 bufferScale = 1;
    std::list<LOutput*> outputs;

    LSize tmpSize;
};

#endif // LTEXTUREVIEWPRIVATE_H
