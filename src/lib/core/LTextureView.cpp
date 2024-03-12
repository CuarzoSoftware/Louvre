#include <private/LTexturePrivate.h>
#include <private/LPainterPrivate.h>
#include <LTextureView.h>
#include <LCompositor.h>

LTextureView::LTextureView(LTexture *texture, LView *parent) : LView(LView::Texture, parent)
{
    setTexture(texture);
}

LTextureView::~LTextureView()
{
    if (m_texture)
        LVectorRemoveOneUnordered(m_texture->imp()->textureViews, this);
}

void LTextureView::setInputRegion(const LRegion *region)
{
    if (region)
    {
        if (m_inputRegion)
            *m_inputRegion = *region;
        else
            m_inputRegion = std::make_unique<LRegion>(*region);
    }
    else
        m_inputRegion.reset();
}

void LTextureView::setTranslucentRegion(const LRegion *region)
{
    if (region)
    {
        if (m_translucentRegion)
            *m_translucentRegion = *region;
        else
            m_translucentRegion = std::make_unique<LRegion>(*region);
    }
    else
        m_translucentRegion.reset();
}

void LTextureView::setTexture(LTexture *texture) noexcept
{
    if (texture == m_texture)
        return;

    if (m_texture)
        LVectorRemoveOneUnordered(m_texture->imp()->textureViews, this);

    m_texture = texture;

    if (m_texture)
    {
        m_textureSerial = m_texture->imp()->serial;
        m_texture->imp()->textureViews.push_back(this);
    }

    updateDimensions();
    damageAll();
}

bool LTextureView::nativeMapped() const noexcept
{
    return m_texture != nullptr;
}

const LPoint &LTextureView::nativePos() const noexcept
{
    return m_nativePos;
}

const LSize &LTextureView::nativeSize() const noexcept
{
    if (m_texture && m_texture->imp()->serial != m_textureSerial)
    {
        m_textureSerial = m_texture->imp()->serial;
        updateDimensions();
    }

    return m_dstSize;
}

Float32 LTextureView::bufferScale() const noexcept
{
    return m_bufferScale;
}

void LTextureView::enteredOutput(LOutput *output) noexcept
{
    LVectorPushBackIfNonexistent(m_outputs, output);
}

void LTextureView::leftOutput(LOutput *output) noexcept
{
    LVectorRemoveOneUnordered(m_outputs, output);
}

const std::vector<LOutput *> &LTextureView::outputs() const noexcept
{
    return m_outputs;
}

bool LTextureView::isRenderable() const noexcept
{
    return true;
}

void LTextureView::requestNextFrame(LOutput *output) noexcept
{
    L_UNUSED(output);
}

const LRegion *LTextureView::damage() const noexcept
{
    return &LRegion::EmptyRegion();
}

const LRegion *LTextureView::translucentRegion() const noexcept
{
    return m_translucentRegion.get();
}

const LRegion *LTextureView::opaqueRegion() const noexcept
{
    return nullptr;
}

const LRegion *LTextureView::inputRegion() const noexcept
{
    return m_inputRegion.get();
}

void LTextureView::paintEvent(const PaintEventParams &params) noexcept
{
    if (!m_texture)
        return;

    params.painter->bindTextureMode({
        .texture = m_texture,
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

void LTextureView::updateDimensions() const noexcept
{
    if (dstSizeEnabled())
        m_dstSize = m_customDstSize;
    else if (m_texture)
    {
        if (LFramebuffer::is90Transform(m_transform))
        {
            m_dstSize.setW(roundf(Float32(m_texture->sizeB().h()) / m_bufferScale));
            m_dstSize.setH(roundf(Float32(m_texture->sizeB().w()) / m_bufferScale));
        }
        else
        {
            m_dstSize.setW(roundf(Float32(m_texture->sizeB().w()) / m_bufferScale));
            m_dstSize.setH(roundf(Float32(m_texture->sizeB().h()) / m_bufferScale));
        }
    }

    if (srcRectEnabled())
        m_srcRect = m_customSrcRect;
    else if (m_texture)
    {
        if (LFramebuffer::is90Transform(m_transform))
        {
            m_srcRect.setW(Float32(m_texture->sizeB().h()) / m_bufferScale);
            m_srcRect.setH(Float32(m_texture->sizeB().w()) / m_bufferScale);
        }
        else
        {
            m_srcRect.setW(Float32(m_texture->sizeB().w()) / m_bufferScale);
            m_srcRect.setH(Float32(m_texture->sizeB().h()) / m_bufferScale);
        }
    }
}
