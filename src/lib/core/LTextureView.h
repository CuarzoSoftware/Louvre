#ifndef LTEXTUREVIEW_H
#define LTEXTUREVIEW_H

#include <LView.h>

class Louvre::LTextureView : public LView
{
public:
    LTextureView(LTexture *texture = nullptr, LView *parent = nullptr);
    ~LTextureView();

    virtual void setNativePosC(const LPoint &pos);
    virtual void setInputRegionC(const LRegion *region);
    virtual void setTranslucentRegionC(const LRegion *region);
    virtual void setBufferScale(Int32 scale);
    virtual void setTexture(LTexture *texture);
    virtual LTexture *texture() const;

    virtual bool nativeMapped() const override;
    virtual const LPoint &nativePosC() const override;
    virtual const LSize &nativeSizeC() const override;
    virtual Int32 bufferScale() const override;
    virtual void enteredOutput(LOutput *output) override;
    virtual void leftOutput(LOutput *output) override;
    virtual const std::list<LOutput*> &outputs() const override;
    virtual bool isRenderable() const override;
    virtual void requestNextFrame(LOutput *output) override;
    virtual const LRegion *damageC() const override;
    virtual const LRegion *translucentRegionC() const override;
    virtual const LRegion *opaqueRegionC() const override;
    virtual const LRegion *inputRegionC() const override;
    virtual void paintRectC(LPainter *p,
                           Int32 srcX, Int32 srcY,
                           Int32 srcW, Int32 srcH,
                           Int32 dstX, Int32 dstY,
                           Int32 dstW, Int32 dstH,
                           Float32 scale,
                           Float32 alpha) override;

    LPRIVATE_IMP(LTextureView)
};

#endif // LTEXTUREVIEW_H
