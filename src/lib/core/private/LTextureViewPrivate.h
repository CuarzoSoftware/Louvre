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

    bool dstSizeEnabled = false;
    LSize dstSize = LSize(1);

    LRGBF customColor;
    bool customColorEnabled = false;

    LSize tmpSize;
};

#endif // LTEXTUREVIEWPRIVATE_H
