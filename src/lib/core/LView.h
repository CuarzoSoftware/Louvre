#ifndef LVIEW_H
#define LVIEW_H

#include <LObject.h>

class Louvre::LView : public LObject
{
public:

    LView(UInt32 type, LView *parent = nullptr);
    virtual ~LView();

    enum Type : UInt32
    {
        Layer = 0,
        Surface = 1,
        Texture = 2,
        Solid = 3
    };

    LScene *scene() const;
    UInt32 type() const;

    void repaint();
    LView *parent() const;
    void setParent(LView *view);
    void insertAfter(LView *prev);
    std::list<LView*> &children() const;

    bool clippingEnabled() const;
    void enableClipping(bool enabled);

    bool inputEnabled() const;
    void enableInput(bool enabled);

    bool scalingEnabled() const;
    void enableScaling(bool enabled);
    void setScaledSizeC(const LSize &size);
    const LSize &scaledSizeC() const;

    bool visible() const;
    void setVisible(bool visible);

    Float32 opacity() const;
    void setOpacity(Float32 opacity);

    virtual const LRegion &inputRegionC() const = 0;
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

LPRIVATE_IMP(LView)
};

#endif // LVIEW_H
