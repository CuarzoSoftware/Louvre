#include <private/LTextureViewPrivate.h>
#include <LTexture.h>
#include <LCompositor.h>
#include <LPainter.h>

LTextureView::LTextureView(LTexture *texture, LView *parent) : LView(LView::Texture, parent)
{
    m_imp = new LTextureViewPrivate();
    imp()->texture = texture;
}

LTextureView::~LTextureView()
{
    if (imp()->inputRegion)
        delete imp()->inputRegion;

    if (imp()->translucentRegion)
        delete imp()->translucentRegion;

    delete m_imp;
}

void LTextureView::setNativePosC(const LPoint &pos)
{
    if (mapped() && pos != imp()->nativePos)
        repaint();

    imp()->nativePos = pos;
}

void LTextureView::setInputRegionC(const LRegion *region)
{
    if (region)
    {
        if (imp()->inputRegion)
            *imp()->inputRegion = *region;
        else
        {
            imp()->inputRegion = new LRegion();
            *imp()->inputRegion = *region;
        }
    }
    else
    {
        if (imp()->inputRegion)
        {
            delete imp()->inputRegion;
            imp()->inputRegion = nullptr;
        }
    }
}

void LTextureView::setTranslucentRegionC(const LRegion *region)
{
    if (region)
    {
        if (imp()->translucentRegion)
            *imp()->translucentRegion = *region;
        else
        {
            imp()->translucentRegion = new LRegion();
            *imp()->translucentRegion = *region;
        }
    }
    else
    {
        if (imp()->translucentRegion)
        {
            delete imp()->translucentRegion;
            imp()->translucentRegion = nullptr;
        }
    }
}

void LTextureView::setBufferScale(Int32 scale)
{
    if (scale < 0)
        scale = 0;

    if (mapped() && scale != imp()->bufferScale)
        repaint();

    imp()->bufferScale = scale;
}

void LTextureView::setTexture(LTexture *texture)
{
    if (mapped() && texture != imp()->texture)
        repaint();

    imp()->texture = texture;
}

LTexture *LTextureView::texture() const
{
    return imp()->texture;
}

bool LTextureView::nativeMapped() const
{
    return imp()->texture != nullptr;
}

const LPoint &LTextureView::nativePosC() const
{
    return imp()->nativePos;
}

const LSize &LTextureView::nativeSizeC() const
{
    if (imp()->texture)
    {
        imp()->tmpSize = imp()->texture->sizeB();

        if (imp()->bufferScale)
            imp()->tmpSize = (imp()->tmpSize*compositor()->globalScale())/imp()->bufferScale;

        return imp()->tmpSize;
    }

    imp()->tmpSize = LSize();
    return imp()->tmpSize;
}

Int32 LTextureView::bufferScale() const
{
    return imp()->bufferScale;
}

void LTextureView::enteredOutput(LOutput *output)
{
    imp()->outputs.remove(output);
    imp()->outputs.push_back(output);
}

void LTextureView::leftOutput(LOutput *output)
{
    imp()->outputs.remove(output);
}

const std::list<LOutput *> &LTextureView::outputs() const
{
    return imp()->outputs;
}

bool LTextureView::isRenderable() const
{
    return true;
}

void LTextureView::requestNextFrame(LOutput *output)
{
    L_UNUSED(output);
}

const LRegion *LTextureView::damageC() const
{
    return &imp()->emptyRegion;
}

const LRegion *LTextureView::translucentRegionC() const
{
    return imp()->translucentRegion;
}

const LRegion *LTextureView::opaqueRegionC() const
{
    return nullptr;
}

const LRegion *LTextureView::inputRegionC() const
{
    return imp()->inputRegion;
}

void LTextureView::paintRectC(LPainter *p,
                              Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                              Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                              Float32 scale, Float32 alpha)
{
    if (!imp()->texture)
        return;

    p->drawTextureC(imp()->texture,
                    srcX, srcY, srcW, srcH,
                    dstX, dstY, dstW, dstH,
                    scale, alpha);
}
