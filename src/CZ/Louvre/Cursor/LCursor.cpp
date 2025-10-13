#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>

#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Louvre/Cursor/LImageCursorSource.h>
#include <CZ/Louvre/Cursor/LShapeCursorSource.h>

#include <CZ/Louvre/Other/cursor.h>

#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Core/Utils/CZRegionUtils.h>
#include <CZ/skia/core/SkRect.h>

using namespace CZ;

LCursor::LCursor() noexcept
{
    compositor()->imp()->cursor.reset(this);

    // Default fallback source

    auto ream { RCore::Get() };
    auto format { ream->mainDevice()->textureFormats().formats().find(DRM_FORMAT_ARGB8888) };
    assert(format != ream->mainDevice()->textureFormats().formats().end());

    RPixelBufferInfo info {};
    info.pixels = (UInt8*)louvre_default_cursor_data();
    info.alphaType = kPremul_SkAlphaType;
    info.format = DRM_FORMAT_ARGB8888;
    info.stride = LOUVRE_DEFAULT_CURSOR_STRIDE;
    info.size.set(LOUVRE_DEFAULT_CURSOR_WIDTH, LOUVRE_DEFAULT_CURSOR_HEIGHT);

    m_image = RImage::MakeFromPixels(info, *format);
    assert(m_image);
    m_hotspot = { 9, 9 };
    m_louvreSource = m_fallbackSource = LCursorSource::Make(m_image, m_hotspot);

    // Sampling surface

    RImageConstraints cons {};
    cons.allocator = ream->mainDevice();
    cons.caps[cons.allocator] = RImageCap::RImageCap_Dst;
    cons.readFormats = {DRM_FORMAT_ARGB8888, DRM_FORMAT_ABGR8888};
    auto surfaceImage { RImage::Make({64, 64}, *format, &cons) };
    assert(surfaceImage);
    m_surface = RSurface::WrapImage(surfaceImage);

    setSize(SkSize(24, 24));
    setSource({});
    setVisible(true);
}

LCursor::~LCursor() noexcept
{
    notifyDestruction();
}

void LCursor::setSource(std::shared_ptr<LCursorSource> source) noexcept
{
    if (!source)
        source = m_fallbackSource;

    bool sourceIsVisible {};

    switch (source->visibility())
    {
    case LCursorSource::Visible:
        sourceIsVisible = true;
        break;
    case LCursorSource::Hidden:
        sourceIsVisible = false;
        break;
    case LCursorSource::Auto:
        sourceIsVisible = isVisible();
        break;
    }

    const bool visibilityChanged { sourceIsVisible != isVisible() };
    m_imageChanged |= source != m_source || source->image() != m_image || source->image()->writeSerial() != m_imageWriteSerial;
    m_posChanged |= source->hotspot() != m_hotspot;

    // TODO: Notify enter/leave to current and prev source?

    if (!visibilityChanged && !m_imageChanged && !m_posChanged)
        return;

    m_source = source;
    m_hotspot = m_source->hotspot();
    m_image = m_source->image();
    m_imageWriteSerial = m_image->writeSerial();
    setVisible(sourceIsVisible);
    updateLater();
}

void LCursor::setFallbackSource(std::shared_ptr<LImageCursorSource> source) noexcept
{
    if (!source)
        source = m_louvreSource;

    m_fallbackSource = source;
}

void LCursor::setShapeAsset(CZCursorShape shape, std::shared_ptr<LImageCursorSource> source) noexcept
{
    m_shapeAssets[(size_t)shape - 1] = source;

    // Update the cursor if the current source references the shape
    if (auto currentSource = m_source->asShape())
    {
        if (currentSource->shape() == shape)
            setSource(m_source);
    }
}

void LCursor::setOutput(LOutput *output) noexcept
{
    bool update { !m_output };
    m_output = output;

    if (update)
    {
        m_imageChanged = true;
        updateLater();
    }
}

void LCursor::updateLater() const noexcept
{
    m_posChanged = true;
    compositor()->imp()->unlockPoll();
}

static void Image2Buffer(std::shared_ptr<RSurface> surface, std::shared_ptr<RImage> image, UInt8 *buffer, SkSize size, CZTransform transform) noexcept
{
    surface->setGeometry({
        .viewport = SkRect::MakeWH(64, 64),
        .dst = SkRect::MakeWH(64, 64),
        .transform = transform});
    auto pass { surface->beginPass(RPassCap_Painter) };
    assert(pass);
    auto *p { pass->getPainter() };
    p->save();
    p->setBlendMode(RBlendMode::Src);
    p->setColor(SK_ColorTRANSPARENT);
    p->clear();
    RDrawImageInfo info {};
    info.image = image;
    info.srcScale = 1.f;
    info.srcTransform = CZTransform::Normal;
    info.magFilter = RImageFilter::Linear;
    info.minFilter = RImageFilter::Linear;
    info.src = SkRect::Make(info.image->size());
    info.dst = SkIRect::MakeSize(size.toRound());
    p->drawImage(info);
    p->restore();
    pass.reset();

    RPixelBufferRegion trans {};
    trans.pixels = buffer;
    trans.region.setRect(SkIRect::MakeWH(64, 64));
    trans.stride = 64 * 4;
    trans.format = DRM_FORMAT_ARGB8888;

    if (!surface->image()->readPixels(trans))
    {
        trans.format = DRM_FORMAT_ABGR8888;

        if (!surface->image()->readPixels(trans))
            return;

        // Convert to RGBA8888
        UInt8 tmp;
        for (Int32 i = 0; i < 64*64*4; i+=4)
        {
            tmp = buffer[i];
            buffer[i] = buffer[i+2];
            buffer[i+2] = tmp;
        }
    }
}

void LCursor::update() noexcept
{
    if (!cursor()->output())
        return;

    if (!m_imageChanged && !m_posChanged)
        return;

    const SkPoint newHotspotS {
        (m_hotspot.x() * m_size.width())/SkScalar(m_image->size().width()),
        (m_hotspot.y() * m_size.height())/SkScalar(m_image->size().height())
    };

    const SkPoint newPosS { cursor()->pos() - newHotspotS };
    m_rect.setXYWH(newPosS.x(), newPosS.y(), m_size.width(), m_size.height());

    std::unordered_set<LOutput*> leave, enter;

    for (LOutput *o : compositor()->outputs())
    {
        if (m_isVisible && SkIRect::Intersects(o->rect(), m_rect))
        {
            const auto it { m_intersectedOutputs.insert(o) };

            if (it.second)
                enter.emplace(o);

            if (hasPlane(o) && (m_imageChanged || it.second))
            {
                if (isPlaneEnabled(o))
                {
                    Image2Buffer(
                        m_surface,
                        m_image,
                        m_buffer,
                        SkSize(m_size.width() * o->fractionalScale(),
                               m_size.height() * o->fractionalScale()),
                        o->transform());

                    o->backend()->setCursor(m_buffer);
                }
                else
                    o->backend()->setCursor(nullptr);
            }
        }
        else
        {
            leave.emplace(o);
            m_intersectedOutputs.erase(o);
            o->backend()->setCursor(nullptr);
        }

        if (cursor()->isPlaneEnabled(o))
        {
            SkPoint p { newPosS - SkPoint::Make(o->pos().x(), o->pos().y()) };

            if (o->transform() == CZTransform::Flipped)
                p.fX = o->rect().width() - p.x() - m_size.width();
            else if (o->transform() == CZTransform::Rotated270)
            {
                const Float32 tmp { p.x() };
                p.fX = o->rect().height() - p.y() - m_size.height();
                p.fY = tmp;
            }
            else if (o->transform() == CZTransform::Rotated180)
            {
                p.fX = o->rect().width() - p.x() - m_size.width();
                p.fY = o->rect().height() - p.y() - m_size.height();
            }
            else if (o->transform() == CZTransform::Rotated90)
            {
                const Float32 tmp { p.x() };
                p.fX = p.y();
                p.fY = o->rect().width() - tmp - m_size.height();
            }
            else if (o->transform() == CZTransform::Flipped270)
            {
                const Float32 tmp { p.x() };
                p.fX = o->rect().height() - p.y() - m_size.height();
                p.fY = o->rect().width() - tmp - m_size.width();
            }
            else if (o->transform() == CZTransform::Flipped180)
                p.fY = o->rect().height() - p.y() - m_size.height();
            else if (o->transform() == CZTransform::Flipped90)
            {
                const Float32 tmp { p.x() };
                p.fX = p.y();
                p.fY = tmp;
            }

            o->backend()->setCursorPos(SkIPoint::Make(
                p.x() * o->fractionalScale(),
                p.y() * o->fractionalScale()));
        }
    }

    m_imageChanged = false;
    m_posChanged = false;

    while (!enter.empty())
    {
        m_source->onEnter(*enter.begin());
        enter.erase(enter.begin());
    }

    while (!leave.empty())
    {
        m_source->onLeave(*leave.begin());
        leave.erase(leave.begin());
    }
}

void CZ::LCursor::setPos(SkPoint pos) noexcept
{
    for (LOutput *output : compositor()->outputs())
    {
        if (output->rect().contains(pos.x(), pos.y()))
        {
            setOutput(output);
            break;
        }
    }

    if (!cursor()->output())
    {
        if (compositor()->outputs().empty())
            return;

        setOutput(compositor()->outputs().front());
    }

    const auto outputRect { cursor()->output()->rect() };

    if (pos.x() > outputRect.right())
        pos.fX = outputRect.right();
    if (pos.x() < outputRect.x())
        pos.fX = outputRect.x();

    if (pos.y() > outputRect.bottom())
        pos.fY = outputRect.bottom();
    if (pos.y() < outputRect.y())
        pos.fY = outputRect.y();

    if (pos != m_pos)
    {
        m_pos = pos;
        m_posChanged = true;
        updateLater();
    }
}

void LCursor::setSize(SkSize size) noexcept
{
    if (size.isEmpty())
        size = {};

    if (m_size != size)
    {
        m_size = size;
        m_imageChanged = true;
        updateLater();
    }
}

void LCursor::setVisible(bool visible) noexcept
{
    if (visible == isVisible())
        return;

    m_isVisible = visible;

    if (!isVisible())
    {
        for (LOutput *o : compositor()->outputs())
            o->backend()->setCursor(nullptr);
    }
    else
    {
        m_imageChanged = true;
        updateLater();
    }
}

void LCursor::repaintOutputs(bool softwareOnly) noexcept
{
    for (auto *o : intersectedOutputs())
        if (!softwareOnly || !isPlaneEnabled(o))
            o->repaint();
}

bool LCursor::hasPlane(const LOutput *output) const noexcept
{
    if (!output) return false;
    return m_surface && output->backend()->hasCursor();
}

void LCursor::enablePlane(LOutput *output, bool enabled) noexcept
{
    if (!output || output->imp()->stateFlags.has(LOutput::LOutputPrivate::CursorPlaneEnabled) == enabled)
        return;

    output->imp()->stateFlags.setFlag(LOutput::LOutputPrivate::CursorPlaneEnabled, enabled);

    if (hasPlane(output))
    {
        m_imageChanged = true;
        updateLater();
        output->repaint();
    }
}

bool LCursor::isPlaneEnabled(const LOutput *output) const noexcept
{
    if (!output)
        return false;

    return output->imp()->stateFlags.has(LOutput::LOutputPrivate::CursorPlaneEnabled) && hasPlane(output);
}

LOutput *LCursor::output() const noexcept
{
    if (m_output)
        return m_output;

    if (!compositor()->outputs().empty())
    {
        m_output = compositor()->outputs().front();
        m_imageChanged = true;
        updateLater();
    }

    return m_output;
}
