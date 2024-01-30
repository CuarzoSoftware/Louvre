#include <private/LTextureViewPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LViewPrivate.h>
#include <private/LPainterPrivate.h>
#include <LTexture.h>
#include <LCompositor.h>

LTextureView::LTextureView(LTexture *texture, LView *parent) :
    LView(LView::Texture, parent),
    LPRIVATE_INIT_UNIQUE(LTextureView)
{
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

void LTextureView::setBufferScale(Float32 scale)
{
    if (scale < 0.5f)
        scale = 0.5f;

    if (mapped() && scale != imp()->bufferScale)
        repaint();

    imp()->bufferScale = scale;
    imp()->updateDimensions();
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
            imp()->textureSerial = imp()->texture->imp()->serial;
            imp()->texture->imp()->textureViews.push_back(this);
            imp()->textureLink = std::prev(imp()->texture->imp()->textureViews.end());
        }

        imp()->updateDimensions();
        damageAll();
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
        imp()->updateDimensions();
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

    if (w == imp()->customDstSize.w() && h == imp()->customDstSize.h())
        return;

    imp()->customDstSize.setW(w);
    imp()->customDstSize.setH(h);
    imp()->updateDimensions();

    if (mapped() && dstSizeEnabled())
        repaint();
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
        damageAll();
    }
}

bool LTextureView::customColorEnabled() const
{
    return imp()->customColorEnabled;
}

void LTextureView::setCustomColor(Float32 r, Float32 g, Float32 b)
{
    if (imp()->customColor.r == r && imp()->customColor.g == g && imp()->customColor.b == b)
        return;

    imp()->customColor.r = r;
    imp()->customColor.g = g;
    imp()->customColor.b = b;

    if (imp()->customColorEnabled)
        damageAll();
}

void LTextureView::setCustomColor(const LRGBF &color)
{
    setCustomColor(color.r, color.g, color.b);
}

const LRGBF &LTextureView::customColor() const
{
    return imp()->customColor;
}

void LTextureView::enableSrcRect(bool enabled)
{
    if (imp()->srcRectEnabled == enabled)
        return;

    imp()->srcRectEnabled = enabled;
    damageAll();
    imp()->updateDimensions();
}

bool LTextureView::srcRectEnabled() const
{
    return imp()->srcRectEnabled;
}

void LTextureView::setSrcRect(const LRectF &srcRect)
{
    if (imp()->customSrcRect == srcRect)
        return;

    imp()->customSrcRect = srcRect;
    imp()->updateDimensions();

    if (imp()->srcRectEnabled)
        damageAll();
}

const LRectF &LTextureView::srcRect() const
{
    return imp()->srcRect;
}

void LTextureView::setTransform(LFramebuffer::Transform transform)
{
    if (imp()->transform == transform)
        return;

    imp()->transform = transform;
    damageAll();
    imp()->updateDimensions();
}

LFramebuffer::Transform LTextureView::transform() const
{
    return imp()->transform;
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

    if (imp()->texture && imp()->texture->imp()->serial != imp()->textureSerial)
    {
        imp()->textureSerial = imp()->texture->imp()->serial;
        imp()->updateDimensions();
    }

    return imp()->dstSize;
}

Float32 LTextureView::bufferScale() const
{
    return imp()->bufferScale;
}

void LTextureView::enteredOutput(LOutput *output)
{
    LVectorPushBackIfNonexistent(imp()->outputs, output);
}

void LTextureView::leftOutput(LOutput *output)
{
    LVectorRemoveOneUnordered(imp()->outputs, output);
}

const std::vector<LOutput *> &LTextureView::outputs() const
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

void LTextureView::paintEvent(const PaintEventParams &params)
{
    if (!imp()->texture)
        return;

    params.painter->bindTextureMode({
        .texture = imp()->texture,
        .pos = pos(),
        .srcRect = srcRect(),
        .dstSize = size(),
        .srcTransform = transform(),
        .srcScale = bufferScale(),
    });

    params.painter->enableCustomTextureColor(customColorEnabled());
    params.painter->setColor(customColor());
    params.painter->drawRegion(*params.region);
}
