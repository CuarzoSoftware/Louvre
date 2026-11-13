#include <climits>
#include <CZ/Ream/RPainter.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureCursorSession.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/ext-image-copy-capture-v1.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureSession.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureFrame.h>
#include <CZ/Louvre/Manager/LImageCaptureManager.h>
#include <CZ/Louvre/Manager/LImageCaptureSession.h>
#include <CZ/Louvre/Manager/LImageCaptureFrame.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Roles/LSubsurfaceRole.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Seat/LOutputMode.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RSurface.h>
#include <algorithm>
#include <cmath>

using namespace CZ;
using namespace CZ::Protocols::ImageCopyCapture;

LImageCaptureSession::LImageCaptureSession(RImageCopyCaptureSession &res, LImageCaptureSourceType type, LOutput *output, LToplevelRole *toplevel, bool paintsCursors,
                                           RImageCopyCaptureCursorSession *cursorRes) noexcept :
    m_res(res),
    m_sourceType(type),
    m_output(output),
    m_toplevel(toplevel),
    m_paintsCursors(paintsCursors),
    m_cursorRes(cursorRes),
    m_isCursor(cursorRes != nullptr)
{
    // The source is gone: nothing to capture anymore
    m_output.setOnDestroyCallback([this](LOutput *) { m_forcingSoftwareCursor = false; stop(); });
    m_toplevel.setOnDestroyCallback([this](LToplevelRole *) { stop(); });
}

LImageCaptureSession::~LImageCaptureSession() noexcept
{
    setForcingSoftwareCursor(false);

    // Its frame (if any) was destroyed first (see RImageCopyCaptureSession)
    m_frame = nullptr;

    if (auto *manager { imageCaptureManager() })
    {
        std::erase(manager->m_sessions, this);
        manager->sessionDestroyed(this);
    }

    notifyDestruction();
}

LClient *LImageCaptureSession::client() const noexcept
{
    return m_res.client();
}

void LImageCaptureSession::accept() noexcept
{
    if (m_state != Pending)
        return;

    m_state = Accepted;
    m_firstFrame = true;
    m_damage.setEmpty();

    if (m_isCursor)
        m_bufferSize = cursorBufferSize();
    else if (m_sourceType == LImageCaptureSourceType::Output)
    {
        if (auto *o { output() })
            m_bufferSize = o->currentMode()->size();

        setForcingSoftwareCursor(m_paintsCursors);
    }
    else
    {
        if (!m_customBufferSize)
        {
            m_bufferSize = {};
            updateToplevelBufferSize(); // Constraints sent below
        }
    }

    if (m_bufferSize.isEmpty())
        m_bufferSize = { 1, 1 };

    sendConstraints();

    // Where the cursor is (cursor sessions)
    if (m_isCursor)
        updateCursor(false);

    // A frame may be waiting already
    captureRequested();
}

void LImageCaptureSession::stop() noexcept
{
    if (m_state == Stopped)
        return;

    m_state = Stopped;
    setForcingSoftwareCursor(false);
    m_res.stopped();

    if (m_frame && m_frame->m_captureRequested && !m_frame->done())
        m_frame->fail(LImageCaptureFrame::Stopped);
}

void LImageCaptureSession::setBufferSize(SkISize size) noexcept
{
    m_customBufferSize = true;

    if (size.isEmpty() || size == m_bufferSize)
        return;

    m_bufferSize = size;
    m_firstFrame = true;
    m_damage.setEmpty();
    m_staging.reset();

    if (m_state != Accepted)
        return;

    sendConstraints();

    // Its buffer no longer matches
    if (m_frame && m_frame->m_captureRequested && !m_frame->done())
        m_frame->fail(LImageCaptureFrame::BufferConstraints);
}

void LImageCaptureSession::addDamage(const SkRegion &region) noexcept
{
    if (m_state == Stopped || region.isEmpty())
        return;

    m_damage.op(region, SkRegion::kUnion_Op);
    m_damage.op(SkIRect::MakeSize(m_bufferSize), SkRegion::kIntersect_Op);

    if (m_state == Accepted)
        captureRequested();
}

void LImageCaptureSession::sendConstraints() noexcept
{
    auto *res { m_res.resource() };
    ext_image_copy_capture_session_v1_send_buffer_size(res, m_bufferSize.width(), m_bufferSize.height());

    // shm: always readable as ARGB8888 (XRGB8888 has the same layout)
    ext_image_copy_capture_session_v1_send_shm_format(res, WL_SHM_FORMAT_ARGB8888);
    ext_image_copy_capture_session_v1_send_shm_format(res, WL_SHM_FORMAT_XRGB8888);

    // dma-buf: formats the main device can render into
    if (auto *device { RCore::Get()->mainDevice() }; device && !device->dmaRenderFormats().formats().empty())
    {
        wl_array array;
        wl_array_init(&array);
        const dev_t id { device->id() };
        auto *dev { static_cast<dev_t*>(wl_array_add(&array, sizeof(dev_t))) };

        if (dev)
        {
            *dev = id;
            ext_image_copy_capture_session_v1_send_dmabuf_device(res, &array);
        }

        wl_array_release(&array);

        for (const auto &format : device->dmaRenderFormats().formats())
        {
            wl_array modifiers;
            wl_array_init(&modifiers);

            for (const auto modifier : format.modifiers())
                if (auto *mod { static_cast<UInt64*>(wl_array_add(&modifiers, sizeof(UInt64))) })
                    *mod = modifier;

            ext_image_copy_capture_session_v1_send_dmabuf_format(res, format.format(), &modifiers);
            wl_array_release(&modifiers);
        }
    }

    ext_image_copy_capture_session_v1_send_done(res);
}

void LImageCaptureSession::captureRequested() noexcept
{
    if (!m_frame || !m_frame->m_captureRequested || m_frame->done())
        return;

    if (m_state == Stopped)
    {
        m_frame->fail(LImageCaptureFrame::Stopped);
        return;
    }

    // Waits until accepted
    if (m_state == Pending)
        return;

    if (!m_frame->validateBuffer())
    {
        m_frame->fail(LImageCaptureFrame::BufferConstraints);
        return;
    }

    // The first frame right away, the next ones once something changes
    if (!m_firstFrame && m_damage.isEmpty())
        return;

    if (m_isCursor)
        renderCursorFrame();
    else if (m_sourceType == LImageCaptureSourceType::Output)
    {
        // Copied after its next paint (see LOutput::LOutputPrivate::blitFramebuffers())
        if (auto *o { output() })
            o->repaint();
        else
            m_frame->fail(LImageCaptureFrame::Stopped);
    }
    else
    {
        if (auto *t { toplevel() })
            t->imageCaptureFrameRequest(m_frame);
        else
            m_frame->fail(LImageCaptureFrame::Stopped);
    }
}

void LImageCaptureSession::updateToplevelBufferSize() noexcept
{
    auto *t { toplevel() };

    if (!t || m_customBufferSize || m_isCursor)
        return;

    // The window geometry (without client side shadows) at the client's scale
    SkISize size { t->windowGeometry().size() };

    if (size.isEmpty())
        size = t->surface()->size();

    const Int32 scale { std::max(1, t->surface()->scale()) };
    size.set(size.width() * scale, size.height() * scale);

    if (size.isEmpty() || size == m_bufferSize)
        return;

    const bool wasSet { !m_bufferSize.isEmpty() };
    m_bufferSize = size;
    m_firstFrame = true;
    m_damage.setEmpty();
    m_staging.reset();

    // Resized: new constraints (the client must reallocate)
    if (wasSet && m_state == Accepted)
    {
        sendConstraints();

        if (m_frame && m_frame->m_captureRequested && !m_frame->done())
            m_frame->fail(LImageCaptureFrame::BufferConstraints);
    }
}

void LImageCaptureSession::captureOutput(bool withCursor) noexcept
{
    if (m_isCursor || m_paintsCursors != withCursor || m_state != Accepted || !m_frame ||
        !m_frame->m_captureRequested || m_frame->done())
        return;

    if (!m_firstFrame && m_damage.isEmpty())
        return;

    auto *o { output() };
    auto src { o ? o->backendImage() : nullptr };

    if (!src)
    {
        m_frame->fail(LImageCaptureFrame::Unknown);
        return;
    }

    // The mode changed: new constraints
    if (src->size() != m_bufferSize)
    {
        m_bufferSize = src->size();
        m_firstFrame = true;
        m_damage.setEmpty();
        sendConstraints();
        m_frame->fail(LImageCaptureFrame::BufferConstraints);
        return;
    }

    const SkIRect full { SkIRect::MakeSize(m_bufferSize) };
    SkRegion region;

    if (m_firstFrame)
        region.setRect(full);
    else
    {
        region = m_damage;
        region.op(m_frame->clientDamage(), SkRegion::kUnion_Op);
        region.op(full, SkRegion::kIntersect_Op);
    }

    if (!m_frame->copyFrom(src, region))
    {
        m_frame->fail(LImageCaptureFrame::Unknown);
        return;
    }

    SkRegion damage;

    if (m_firstFrame)
        damage.setRect(full);
    else
        damage = m_damage;

    m_frame->sendReady(damage, o->transform());
}

void LImageCaptureSession::setForcingSoftwareCursor(bool forcing) noexcept
{
    if (m_forcingSoftwareCursor == forcing)
        return;

    m_forcingSoftwareCursor = forcing;

    auto *o { output() };

    if (!o)
        return;

    // While > 0 the cursor plane is off (see LCursor::isPlaneEnabled()): the cursor is painted
    // in software, so it's part of the captured frames and of their damage.
    o->imp()->softwareCursorCaptures += forcing ? 1 : -1;
    cursor()->m_imageChanged = true;
    cursor()->updateLater();
    o->repaint();
}

void LImageCaptureSession::SurfaceCommitted(LSurface &surface) noexcept
{
    auto *manager { imageCaptureManager() };

    if (!manager || manager->sessions().empty())
        return;

    // The toplevel it belongs to, and its position relative to it
    LSurface *root { &surface };
    SkIPoint offset { 0, 0 };

    while (root->subsurface() && root->parent())
    {
        offset += root->subsurface()->localPos();
        root = root->parent();
    }

    LToplevelRole *toplevel { root->toplevel() };

    if (!toplevel)
        return;

    // A copy: the compositor's code (frame requests) may change sessions()
    const auto sessions { manager->sessions() };

    for (auto *session : sessions)
    {
        if (session->toplevel() != toplevel || session->state() == Stopped || session->m_isCursor)
            continue;

        session->updateToplevelBufferSize();

        if (session->m_customBufferSize)
        {
            // Unknown mapping (rendered by the compositor): everything may have changed
            session->addDamage(SkRegion(SkIRect::MakeSize(session->m_bufferSize)));
            continue;
        }

        // Surface damage -> window geometry -> buffer pixels
        const SkIPoint origin { offset - toplevel->windowGeometry().topLeft() };
        const Float32 scale { Float32(std::max(1, toplevel->surface()->scale())) };
        SkRegion damage;
        SkRegion::Iterator it { surface.damage() };

        while (!it.done())
        {
            const SkIRect r { it.rect().makeOffset(origin.x(), origin.y()) };
            damage.op(SkIRect::MakeLTRB(
                std::floor(r.left() * scale) - 1,
                std::floor(r.top() * scale) - 1,
                std::ceil(r.right() * scale) + 1,
                std::ceil(r.bottom() * scale) + 1), SkRegion::kUnion_Op);
            it.next();
        }

        session->addDamage(damage);
    }
}

/********************************* CURSOR SESSIONS *********************************/

Float32 LImageCaptureSession::sourceScale() const noexcept
{
    if (auto *o { output() })
        return o->fractionalScale();

    if (auto *t { toplevel() })
        return Float32(std::max(1, t->surface()->scale()));

    return 1.f;
}

SkISize LImageCaptureSession::cursorBufferSize() const noexcept
{
    // The cursor at the source's scale
    const Float32 scale { sourceScale() };
    const SkSize size { cursor()->size() };
    return { std::max(1, Int32(std::ceil(size.width() * scale))), std::max(1, Int32(std::ceil(size.height() * scale))) };
}

void LImageCaptureSession::CursorChanged(bool imageChanged) noexcept
{
    auto *manager { imageCaptureManager() };

    if (!manager)
        return;

    const auto sessions { manager->sessions() };

    for (auto *session : sessions)
        if (session->m_isCursor && session->m_state == Accepted)
            session->updateCursor(imageChanged);
}

void LImageCaptureSession::updateCursor(bool imageChanged) noexcept
{
    auto *cursorRes { m_cursorRes.get() };
    const Float32 scale { sourceScale() };

    // New size: new constraints (the client must reallocate)
    if (const SkISize size { cursorBufferSize() }; size != m_bufferSize)
    {
        m_bufferSize = size;
        m_firstFrame = true;
        m_damage.setEmpty();
        m_staging.reset();
        sendConstraints();

        if (m_frame && m_frame->m_captureRequested && !m_frame->done())
            m_frame->fail(LImageCaptureFrame::BufferConstraints);
    }
    else if (imageChanged)
        addDamage(SkRegion(SkIRect::MakeSize(m_bufferSize)));

    if (!cursorRes)
        return;

    // The captured area and where the cursor is, in its buffer pixels
    SkIRect area {};
    SkPoint hotspotPos { cursor()->pos() };
    bool inside { false };

    if (auto *o { output() })
    {
        area = o->rect();
        const SkPoint local { hotspotPos.x() - area.x(), hotspotPos.y() - area.y() };

        // Transformed buffer pixels (like the frames of the output)
        SkPoint p { local };
        const Float32 w { Float32(area.width()) }, h { Float32(area.height()) };

        switch (o->transform())
        {
        case CZTransform::Rotated90:   p = { local.y(), w - local.x() }; break;
        case CZTransform::Rotated180:  p = { w - local.x(), h - local.y() }; break;
        case CZTransform::Rotated270:  p = { h - local.y(), local.x() }; break;
        case CZTransform::Flipped:     p = { w - local.x(), local.y() }; break;
        case CZTransform::Flipped90:   p = { local.y(), local.x() }; break;
        case CZTransform::Flipped180:  p = { local.x(), h - local.y() }; break;
        case CZTransform::Flipped270:  p = { h - local.y(), w - local.x() }; break;
        default: break;
        }

        hotspotPos = { p.x() * scale, p.y() * scale };
    }
    else if (auto *t { toplevel() })
    {
        // The window geometry on screen
        const SkIPoint origin { t->surface()->rolePos() + t->windowGeometry().topLeft() };
        area = SkIRect::MakePtSize(origin, t->windowGeometry().size());
        hotspotPos = { (hotspotPos.x() - origin.x()) * scale, (hotspotPos.y() - origin.y()) * scale };
    }

    inside = cursor()->isVisible() && SkIRect::Intersects(area, cursor()->rect());

    if (!inside)
    {
        if (m_cursorInside)
        {
            m_cursorInside = false;
            cursorRes->leave();
        }

        return;
    }

    if (!m_cursorInside)
    {
        m_cursorInside = true;
        m_cursorPos = m_cursorHotspot = { INT32_MIN, INT32_MIN };
        cursorRes->enter();
    }

    const SkIPoint pos { SkScalarRoundToInt(hotspotPos.x()), SkScalarRoundToInt(hotspotPos.y()) };

    if (pos != m_cursorPos)
    {
        m_cursorPos = pos;
        cursorRes->position(pos.x(), pos.y());
    }

    // The hotspot in the cursor image (frames), in buffer pixels
    SkIPoint hotspot { 0, 0 };

    if (auto image { cursor()->m_image }; image && !image->size().isEmpty())
        hotspot = {
            SkScalarRoundToInt(Float32(cursor()->m_hotspot.x()) * Float32(m_bufferSize.width()) / Float32(image->size().width())),
            SkScalarRoundToInt(Float32(cursor()->m_hotspot.y()) * Float32(m_bufferSize.height()) / Float32(image->size().height())) };

    if (hotspot != m_cursorHotspot)
    {
        m_cursorHotspot = hotspot;
        cursorRes->hotspot(hotspot.x(), hotspot.y());
    }
}

void LImageCaptureSession::renderCursorFrame() noexcept
{
    auto target { m_frame->surface() };

    if (!target)
    {
        m_frame->fail(LImageCaptureFrame::Unknown);
        return;
    }

    // Always the whole (small) image: the cursor image at the buffer size
    const SkIRect full { SkIRect::MakeSize(m_bufferSize) };
    const SkRegion region { full };

    {
        auto pass { target->beginPass(RPassCap_Painter) };

        if (!pass)
        {
            m_frame->fail(LImageCaptureFrame::Unknown);
            return;
        }

        auto *painter { pass->getPainter() };
        painter->setBlendMode(RBlendMode::Src);
        painter->setColor(SK_ColorTRANSPARENT);
        painter->drawColor(region);

        if (auto image { cursor()->m_image }; image && cursor()->isVisible())
        {
            painter->setBlendMode(RBlendMode::SrcOver);
            RDrawImageInfo info {};
            info.image = image;
            info.src = SkRect::Make(SkIRect::MakeSize(image->size()));
            info.srcScale = 1.f;
            info.dst = full;
            info.minFilter = info.magFilter = RImageFilter::Linear;
            painter->drawImage(info, &region);
        }
    }

    // Everything changed
    m_firstFrame = true;
    m_frame->ready();
}
