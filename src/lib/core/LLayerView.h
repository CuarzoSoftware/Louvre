#ifndef LLAYERVIEW_H
#define LLAYERVIEW_H

#include <LView.h>

class Louvre::LLayerView : public LView
{
public:
    LLayerView(LView *parent = nullptr);
    ~LLayerView();

    virtual void setCustomPosC(const LPoint &pos);
    virtual const LPoint &customPosC() const;
    virtual void setCustomSize(const LSize &size);
    virtual void setInputRegionC(const LRegion &region) const;

    virtual const LRegion *inputRegionC() const override;
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
    virtual void paintRect(LPainter *p,
                           Int32 srcX, Int32 srcY,
                           Int32 srcW, Int32 srcH,
                           Int32 dstX, Int32 dstY,
                           Int32 dstW, Int32 dstH,
                           Float32 scale,
                           Float32 alpha) override;

LPRIVATE_IMP(LLayerView)
};

#endif // LLAYERVIEW_H
