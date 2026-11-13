#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureFrame.h>
#include <CZ/Louvre/Manager/LImageCaptureSession.h>
#include <CZ/Louvre/Manager/LImageCaptureFrame.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RPainter.h>
#include <CZ/Ream/RSync.h>
#include <drm_fourcc.h>
#include <ctime>

using namespace CZ;
using namespace CZ::Protocols::ImageCopyCapture;

LImageCaptureFrame::LImageCaptureFrame(RImageCopyCaptureFrame &res, LImageCaptureSession &session) noexcept :
    m_res(res),
    m_session(session)
{
    m_session.m_frame = this;
}

LImageCaptureFrame::~LImageCaptureFrame() noexcept
{
    if (m_session.m_frame == this)
        m_session.m_frame = nullptr;

    notifyDestruction();
}

static void SetPixelGeometry(RSurface &surface, SkISize size) noexcept
{
    surface.setGeometry({
        .viewport = SkRect::Make(SkIRect::MakeSize(size)),
        .dst = SkRect::Make(SkIRect::MakeSize(size)),
        .transform = CZTransform::Normal });
}

bool LImageCaptureFrame::validateBuffer() noexcept
{
    wl_resource *buffer { m_res.buffer() };

    if (!buffer)
        return false;

    const SkISize size { m_session.bufferSize() };

    if (auto *shm { wl_shm_buffer_get(buffer) })
    {
        const auto format { wl_shm_buffer_get_format(shm) };
        return wl_shm_buffer_get_width(shm) == size.width() &&
               wl_shm_buffer_get_height(shm) == size.height() &&
               (format == WL_SHM_FORMAT_ARGB8888 || format == WL_SHM_FORMAT_XRGB8888);
    }

    if (auto image { dmaImage() })
        return image->size() == size;

    return false;
}

std::shared_ptr<RImage> LImageCaptureFrame::dmaImage() const noexcept
{
    wl_resource *buffer { m_res.buffer() };

    if (!buffer || !LDMABuffer::isDMABuffer(buffer))
        return {};

    return static_cast<LDMABuffer*>(wl_resource_get_user_data(buffer))->image();
}

std::shared_ptr<RSurface> LImageCaptureFrame::surface() noexcept
{
    if (m_surface)
        return m_surface;

    const SkISize size { m_session.bufferSize() };

    // dma-buf: rendered right into the client's buffer
    if (auto image { dmaImage() })
    {
        m_surface = RSurface::WrapImage(image);

        if (m_surface)
            SetPixelGeometry(*m_surface, size);

        return m_surface;
    }

    // shm: rendered into an image of the session (keeping the full content), then copied
    auto &staging { m_session.m_staging };

    if (!staging || staging->image()->size() != size)
    {
        staging = RSurface::Make(size, 1.f, true);
        m_session.m_stagingFresh = true;
    }

    if (staging)
        SetPixelGeometry(*staging, size);

    m_surface = staging;
    return m_surface;
}

SkRegion LImageCaptureFrame::paintRegion() noexcept
{
    const SkIRect full { SkIRect::MakeSize(m_session.bufferSize()) };
    auto target { surface() };
    SkRegion region;

    if (!target || m_session.m_firstFrame)
        region.setRect(full);
    else if (target == m_session.m_staging)
    {
        // Up to date except for what changed
        if (m_session.m_stagingFresh)
            region.setRect(full);
        else
            region = m_session.damage();
    }
    else
    {
        // The client's buffer: also what it says is stale
        region = m_session.damage();
        region.op(m_clientDamage, SkRegion::kUnion_Op);
    }

    region.op(full, SkRegion::kIntersect_Op);
    return region;
}

void LImageCaptureFrame::ready(CZTransform transform) noexcept
{
    if (m_done)
        return;

    const SkIRect full { SkIRect::MakeSize(m_session.bufferSize()) };

    // shm: copy the staging content
    if (m_surface && m_surface == m_session.m_staging)
    {
        SkRegion region;

        if (m_session.m_firstFrame)
            region.setRect(full);
        else
        {
            region = m_session.damage();
            region.op(m_clientDamage, SkRegion::kUnion_Op);
            region.op(full, SkRegion::kIntersect_Op);
        }

        if (!copyFrom(m_surface->image(), region))
        {
            fail(Unknown);
            return;
        }
    }
    else if (m_surface)
    {
        // dma-buf: the client reads it right after `ready`
        if (auto sync { RSync::Make() })
            sync->cpuWait();
    }

    SkRegion damage;

    if (m_session.m_firstFrame)
        damage.setRect(full);
    else
        damage = m_session.damage();

    m_session.m_stagingFresh = false;
    sendReady(damage, transform);
}

void LImageCaptureFrame::fail(FailReason reason) noexcept
{
    if (m_done)
        return;

    m_done = true;
    m_res.failed(static_cast<UInt32>(reason));
}

bool LImageCaptureFrame::copyFrom(std::shared_ptr<RImage> src, const SkRegion &region) noexcept
{
    if (!src)
        return false;

    if (region.isEmpty())
        return true;

    const SkISize size { m_session.bufferSize() };
    wl_resource *buffer { m_res.buffer() };

    if (!buffer)
        return false;

    // shm: read the pixels into the client's memory
    if (auto *shm { wl_shm_buffer_get(buffer) })
    {
        const auto readInto = [&](RImage &image) -> bool {
            wl_shm_buffer_begin_access(shm);
            RPixelBufferRegion info {};
            info.pixels = static_cast<UInt8*>(wl_shm_buffer_get_data(shm));
            info.stride = wl_shm_buffer_get_stride(shm);
            info.format = DRM_FORMAT_ARGB8888; // Same layout as XRGB8888
            info.region = region;
            const bool ok { image.readPixels(info) };
            wl_shm_buffer_end_access(shm);
            return ok;
        };

        if (readInto(*src))
            return true;

        // Not readable (e.g. a scanout image): draw it into a readable image first
        auto &staging { m_session.m_staging };

        if (!staging || staging->image()->size() != size)
        {
            staging = RSurface::Make(size, 1.f, true);
            m_session.m_stagingFresh = true;
        }

        if (!staging || staging->image() == src)
            return false;

        SetPixelGeometry(*staging, size);

        {
            auto pass { staging->beginPass(RPassCap_Painter) };

            if (!pass)
                return false;

            auto *painter { pass->getPainter() };
            painter->setBlendMode(RBlendMode::Src);
            RDrawImageInfo info {};
            info.image = src;
            info.src = SkRect::Make(SkIRect::MakeSize(size));
            info.srcScale = 1.f;
            info.dst = SkIRect::MakeSize(size);
            info.minFilter = info.magFilter = RImageFilter::Nearest;
            painter->drawImage(info, &region);
        }

        return readInto(*staging->image());
    }

    // dma-buf: draw into the client's buffer
    auto target { surface() };

    if (!target)
        return false;

    {
        auto pass { target->beginPass(RPassCap_Painter) };

        if (!pass)
            return false;

        auto *painter { pass->getPainter() };
        painter->setBlendMode(RBlendMode::Src);
        RDrawImageInfo info {};
        info.image = src;
        info.src = SkRect::Make(SkIRect::MakeSize(size));
        info.srcScale = 1.f;
        info.dst = SkIRect::MakeSize(size);
        info.minFilter = info.magFilter = RImageFilter::Nearest;
        painter->drawImage(info, &region);
    }

    // The client reads it right after `ready`
    if (auto sync { RSync::Make() })
        sync->cpuWait();

    return true;
}

void LImageCaptureFrame::sendReady(const SkRegion &damage, CZTransform transform) noexcept
{
    m_res.transform(transform);

    SkRegion::Iterator it { damage };

    while (!it.done())
    {
        m_res.damage(it.rect());
        it.next();
    }

    timespec now {};
    clock_gettime(CLOCK_MONOTONIC, &now);
    m_res.presentationTime(now);
    m_res.ready();

    m_done = true;
    m_session.m_damage.setEmpty();
    m_session.m_firstFrame = false;
}
