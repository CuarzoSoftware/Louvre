#ifndef LSOLIDCOLORVIEW_H
#define LSOLIDCOLORVIEW_H

#include <LView.h>

class Louvre::LSolidColorView : public LView
{
public:
    LSolidColorView(LView *parent);
    LSolidColorView(Float32 r = 0.f, Float32 g = 0.f, Float32 b = 0.f, Float32 a = 1.f, LView *parent = nullptr);
    LSolidColorView(const LRGBF &color, Float32 a = 1.f, LView *parent = nullptr);
    ~LSolidColorView();

    void setColor(const LRGBF &color);
    void setColor(Float32 r, Float32 g, Float32 b);
    const LRGBF &color() const;

    virtual void setPosC(const LPoint &pos);

    virtual bool mapped() const override;
    virtual const LPoint &posC() const override;
    virtual const LSize &sizeC() const override;
    virtual Int32 scale() const override;
    virtual void enterOutput(LOutput *output) override;
    virtual void leaveOutput(LOutput *output) override;
    virtual const std::list<LOutput*> &outputs() const override;
    virtual bool isRenderable() const override;
    virtual bool forceRequestNextFrameEnabled() const override;
    virtual void enableForceRequestNextFrame(bool enabled) override;
    virtual void requestNextFrame(LOutput *output) override;
    virtual const LRegion *damageC() const override;
    virtual const LRegion *translucentRegionC() const override;
    virtual const LRegion *opaqueRegionC() const override;
    virtual const LRegion *inputRegionC() const override;
    virtual void paintRect(LPainter *p,
                           Int32 srcX, Int32 srcY,
                           Int32 srcW, Int32 srcH,
                           Int32 dstX, Int32 dstY,
                           Int32 dstW, Int32 dstH,
                           Float32 scale,
                           Float32 alpha) override;

LPRIVATE_IMP(LSolidColorView)
};

#endif // LSOLIDCOLORVIEW_H
