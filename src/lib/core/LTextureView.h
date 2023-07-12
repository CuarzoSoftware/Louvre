#ifndef LTEXTUREVIEW_H
#define LTEXTUREVIEW_H

#include <LView.h>

class Louvre::LTextureView : public LView
{
public:
    LTextureView(LTexture *texture = nullptr, LView *parent = nullptr);

    /*
    virtual bool mapped() const = 0;
    virtual const LPoint &posC() const = 0;
    virtual const LSize &sizeC() const = 0;
    virtual Int32 scale() const = 0;
    virtual void enterOutput(LOutput *output) = 0;
    virtual void leaveOutput(LOutput *output) = 0;
    virtual const std::list<LOutput*> &outputs() const = 0;
    virtual bool isRenderable() const = 0;
    virtual bool forceRequestNextFrameEnabled() const = 0;
    virtual void enableForceRequestNextFrame(bool enabled) = 0;
    virtual void requestNextFrame(LOutput *output) = 0;
    virtual const LRegion *damageC() const = 0;
    virtual const LRegion *translucentRegionC() const = 0;
    virtual const LRegion *opaqueRegionC() const = 0;
    virtual const LRegion *inputRegionC() const = 0;
    virtual void paintRect(LPainter *p,
                           Int32 srcX, Int32 srcY,
                           Int32 srcW, Int32 srcH,
                           Int32 dstX, Int32 dstY,
                           Int32 dstW, Int32 dstH,
                           Float32 scale,
                           Float32 alpha) = 0;*/

    LPRIVATE_IMP(LTextureView)
};

#endif // LTEXTUREVIEW_H
