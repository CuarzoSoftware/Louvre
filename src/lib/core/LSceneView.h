#ifndef LSCENEVIEW_H
#define LSCENEVIEW_H

#include <LView.h>

class Louvre::LSceneView : public LView
{
public:
    LSceneView(const LSize &sizeB, Int32 bufferScale, LView *parent = nullptr);
    ~LSceneView();

    const LRGBAF &clearColor() const;
    void setClearColor(Float32 r, Float32 g, Float32 b, Float32 a);
    void setClearColor(const LRGBAF &color);
    void damageAll(LOutput *output);
    void addDamage(LOutput *output, const LRegion &damage);

    bool isLScene() const;

    virtual void render(const LRegion *exclude = nullptr);
    virtual const LTexture *texture(Int32 index = 0) const;

    // Render buffer
    void setPos(const LPoint &pos);
    void setSizeB(const LSize &size);
    void setScale(Int32 scale);

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

LPRIVATE_IMP(LSceneView)
friend class LScene;
LSceneView(LFramebuffer *framebuffer = nullptr, LView *parent = nullptr);
};

#endif // LSCENEVIEW_H
