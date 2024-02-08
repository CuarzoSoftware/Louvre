#ifndef LTEXTUREVIEWPRIVATE_H
#define LTEXTUREVIEWPRIVATE_H

#include <LTexture.h>
#include <LTextureView.h>
#include <LRegion.h>

using namespace Louvre;

LPRIVATE_CLASS(LTextureView)
    LTexture *texture = nullptr;
    LRegion *inputRegion = nullptr;
    LRegion *translucentRegion = nullptr;
    LRegion emptyRegion;
    LPoint nativePos;
    Float32 bufferScale = 1.f;
    std::vector<LOutput*> outputs;

    bool dstSizeEnabled = false;
    LSize dstSize = LSize(1);
    LSize customDstSize = LSize(1);

    bool srcRectEnabled = false;
    LRectF customSrcRect = LRectF(0, 0, 1, 1);
    LRectF srcRect = LRectF(0, 0, 1, 1);

    LFramebuffer::Transform transform = LFramebuffer::Normal;

    LRGBF customColor;
    bool customColorEnabled = false;

    UInt32 textureSerial = 0;

    inline void updateDimensions()
    {
        if (dstSizeEnabled)
            dstSize = customDstSize;
        else if (texture)
        {
            if (LFramebuffer::is90Transform(transform))
            {
                dstSize.setW(roundf(Float32(texture->sizeB().h()) / bufferScale));
                dstSize.setH(roundf(Float32(texture->sizeB().w()) / bufferScale));
            }
            else
            {
                dstSize.setW(roundf(Float32(texture->sizeB().w()) / bufferScale));
                dstSize.setH(roundf(Float32(texture->sizeB().h()) / bufferScale));
            }
        }

        if (srcRectEnabled)
            srcRect = customSrcRect;
        else if (texture)
        {
            if (LFramebuffer::is90Transform(transform))
            {
                srcRect.setW(Float32(texture->sizeB().h()) / bufferScale);
                srcRect.setH(Float32(texture->sizeB().w()) / bufferScale);
            }
            else
            {
                srcRect.setW(Float32(texture->sizeB().w()) / bufferScale);
                srcRect.setH(Float32(texture->sizeB().h()) / bufferScale);
            }
        }
    }
};

#endif // LTEXTUREVIEWPRIVATE_H
