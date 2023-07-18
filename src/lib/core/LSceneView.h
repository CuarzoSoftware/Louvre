#ifndef LSCENEVIEW_H
#define LSCENEVIEW_H

#include <LView.h>

class Louvre::LSceneView : public LView
{
public:
    LSceneView(LFramebuffer *framebuffer = nullptr, LView *parent = nullptr);

    const LRGBF &clearColor() const;
    void setClearColor(Float32 r, Float32 g, Float32 b);
    void setClearColor(const LRGBF &color);
    void damageAll(LOutput *output);
    void addDamageC(LOutput *output, const LRegion &damage);

    virtual void render(LOutput *output);

    virtual bool customPosEnabled() const;
    virtual void enableCustomPos(bool enabled);
    virtual void setCustomPosC(const LPoint &pos);
    virtual const LPoint &customPosC() const;

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

LPRIVATE_IMP(LSceneView)
};

#endif // LSCENEVIEW_H
