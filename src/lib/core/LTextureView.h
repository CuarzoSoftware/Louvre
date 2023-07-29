#ifndef LTEXTUREVIEW_H
#define LTEXTUREVIEW_H

#include <LView.h>

class Louvre::LTextureView : public LView
{
public:
    LTextureView(LTexture *texture = nullptr, LView *parent = nullptr);
    ~LTextureView();

    virtual void setPos(Int32 x, Int32 y);
    void setPos(const LPoint &pos);
    virtual void setInputRegion(const LRegion *region);
    virtual void setTranslucentRegion(const LRegion *region);
    virtual void setBufferScale(Int32 scale);
    virtual void setTexture(LTexture *texture);
    virtual LTexture *texture() const;

    virtual void enableDstSize(bool enabled);
    virtual bool dstSizeEnabled() const;
    virtual void setDstSize(Int32 w, Int32 h);
    void setDstSize(const LSize &dstSize);

    virtual bool nativeMapped() const override;
    virtual const LPoint &nativePos() const override;
    virtual const LSize &nativeSize() const override;
    virtual Int32 bufferScale() const override;
    virtual void enteredOutput(LOutput *output) override;
    virtual void leftOutput(LOutput *output) override;
    virtual const std::list<LOutput*> &outputs() const override;
    virtual bool isRenderable() const override;
    virtual void requestNextFrame(LOutput *output) override;
    virtual const LRegion *damage() const override;
    virtual const LRegion *translucentRegion() const override;
    virtual const LRegion *opaqueRegion() const override;
    virtual const LRegion *inputRegion() const override;
    virtual void paintRect(LPainter *p,
                           Int32 srcX, Int32 srcY,
                           Int32 srcW, Int32 srcH,
                           Int32 dstX, Int32 dstY,
                           Int32 dstW, Int32 dstH,
                           Float32 scale,
                           Float32 alpha) override;

    LPRIVATE_IMP(LTextureView)
};

#endif // LTEXTUREVIEW_H
