#include <private/LTextureViewPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LViewPrivate.h>
#include <private/LPainterPrivate.h>
#include <LTexture.h>
#include <LCompositor.h>

LTextureView::LTextureView(LTexture *texture, LView *parent) : LView(LView::Texture, parent)
{
    m_imp = new LTextureViewPrivate();
    setTexture(texture);
}

LTextureView::~LTextureView()
{
    if (imp()->texture)
        imp()->texture->imp()->textureViews.erase(imp()->textureLink);

    if (imp()->inputRegion)
        delete imp()->inputRegion;

    if (imp()->translucentRegion)
        delete imp()->translucentRegion;

    delete m_imp;
}

void LTextureView::setPos(Int32 x, Int32 y)
{
    if (mapped() && (x != imp()->nativePos.x() || y != imp()->nativePos.y()))
        repaint();

    imp()->nativePos.setX(x);
    imp()->nativePos.setY(y);
}

void LTextureView::setPos(const LPoint &pos)
{
    setPos(pos.x(), pos.y());
}

void LTextureView::setInputRegion(const LRegion *region)
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

void LTextureView::setTranslucentRegion(const LRegion *region)
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
    if (texture != imp()->texture)
    {
        if (imp()->texture)
            imp()->texture->imp()->textureViews.erase(imp()->textureLink);

        imp()->texture = texture;

        if (imp()->texture)
        {
            imp()->texture->imp()->textureViews.push_back(this);
            imp()->textureLink = std::prev(imp()->texture->imp()->textureViews.end());
        }

        LView *nativeView = this;

        nativeView->imp()->markAsChangedOrder(false);

        if (mapped())
            repaint();
    }
}

LTexture *LTextureView::texture() const
{
    return imp()->texture;
}

void LTextureView::enableDstSize(bool enabled)
{
    if (enabled != imp()->dstSizeEnabled)
    {
        imp()->dstSizeEnabled = enabled;
        repaint();
    }
}

bool LTextureView::dstSizeEnabled() const
{
    return imp()->dstSizeEnabled;
}

void LTextureView::setDstSize(Int32 w, Int32 h)
{
    if (w < 0)
        w = 0;

    if (h < 0)
        h = 0;

    if (imp()->dstSizeEnabled && (w != imp()->dstSize.w() || h != imp()->dstSize.h()))
        repaint();

    imp()->dstSize.setW(w);
    imp()->dstSize.setH(h);
}

void LTextureView::setDstSize(const LSize &dstSize)
{
    setDstSize(dstSize.w(), dstSize.h());
}

void LTextureView::enableCustomColor(bool enabled)
{
    if (imp()->customColorEnabled != enabled)
    {
        imp()->customColorEnabled = enabled;

        LView *nativeView = this;

        nativeView->imp()->markAsChangedOrder(false);

        if (mapped())
            repaint();
    }
}

bool LTextureView::customColorEnabled() const
{
    return imp()->customColorEnabled;
}

void LTextureView::setCustomColor(Float32 r, Float32 g, Float32 b)
{
    if (imp()->customColorEnabled)
    {
        if (imp()->customColor.r != r || imp()->customColor.g != g || imp()->customColor.b != b)
        {
            LView *nativeView = this;

            nativeView->imp()->markAsChangedOrder(false);

            if (mapped())
                repaint();
        }
    }

    imp()->customColor.r = r;
    imp()->customColor.g = g;
    imp()->customColor.b = b;
}

void LTextureView::setCustomColor(const LRGBF &color)
{
    setCustomColor(color.r, color.g, color.b);
}

const LRGBF &LTextureView::customColor() const
{
    return imp()->customColor;
}

bool LTextureView::nativeMapped() const
{
    return imp()->texture != nullptr;
}

const LPoint &LTextureView::nativePos() const
{
    return imp()->nativePos;
}

const LSize &LTextureView::nativeSize() const
{
    if (imp()->texture)
    {
        if (imp()->dstSizeEnabled)
            return imp()->dstSize;

        imp()->tmpSize = imp()->texture->sizeB();

        if (imp()->bufferScale)
            imp()->tmpSize = imp()->tmpSize/imp()->bufferScale;

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

const LRegion *LTextureView::damage() const
{
    return &imp()->emptyRegion;
}

const LRegion *LTextureView::translucentRegion() const
{
    return imp()->translucentRegion;
}

const LRegion *LTextureView::opaqueRegion() const
{
    return nullptr;
}

const LRegion *LTextureView::inputRegion() const
{
    return imp()->inputRegion;
}

void LTextureView::paintRect(LPainter *p,
                              Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                              Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                              Float32 scale, Float32 alpha)
{
    if (!imp()->texture)
        return;

    if (imp()->dstSizeEnabled)
    {
        LSizeF scaling;
        scaling.setW(Float32(imp()->texture->sizeB().w()) / Float32(imp()->dstSize.w() * bufferScale()));
        scaling.setH(Float32(imp()->texture->sizeB().h()) / Float32(imp()->dstSize.h() * bufferScale()));

        if (imp()->customColorEnabled)
        {
            p->imp()->drawColorTexture(imp()->texture,
                                imp()->customColor.r,
                                imp()->customColor.g,
                                imp()->customColor.b,
                                srcX * scaling.x(),
                                srcY * scaling.y(),
                                srcW * scaling.x(),
                                srcH * scaling.y(),
                                dstX,
                                dstY,
                                dstW,
                                dstH,
                                scale, alpha);
        }
        else
        {
            p->imp()->drawTexture(imp()->texture,
                           srcX * scaling.x(),
                           srcY * scaling.y(),
                           srcW * scaling.x(),
                           srcH * scaling.y(),
                           dstX,
                           dstY,
                           dstW,
                           dstH,
                           scale, alpha);
        }
    }
    else
    {
        if (imp()->customColorEnabled)
        {
            p->imp()->drawColorTexture(imp()->texture,
                                imp()->customColor.r,
                                imp()->customColor.g,
                                imp()->customColor.b,
                                srcX, srcY, srcW, srcH,
                                dstX, dstY, dstW, dstH,
                                scale, alpha);
        }
        else
        {
            p->imp()->drawTexture(imp()->texture,
                           srcX, srcY, srcW, srcH,
                           dstX, dstY, dstW, dstH,
                           scale, alpha);
        }
    }
}
