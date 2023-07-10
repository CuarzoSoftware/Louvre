#ifndef LSURFACEVIEW_H
#define LSURFACEVIEW_H

#include <LView.h>

class Louvre::LSurfaceView : public LView
{
public:
    LSurfaceView(LSurface *surface, bool primary = true, bool useCustomPos = false, LView *parent = nullptr);
    ~LSurfaceView();

    LSurface *surface() const;

    bool primary() const;
    void setPrimary(bool primary) const;

    bool customPosEnabled() const;
    void enableCustomPos(bool enable) const;

    // LSurfaceView
    virtual void setCustomPosC(const LPoint &customPosC);
    virtual const LPoint &customPosC();

    // LView
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


LPRIVATE_IMP(LSurfaceView)
};

#endif // LSURFACEVIEW_H
