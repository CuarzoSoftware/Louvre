#include <CZ/Louvre/Protocols/ImageCopyCapture/ext-image-copy-capture-v1.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureFrame.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureSession.h>
#include <CZ/Louvre/Manager/LImageCaptureSession.h>
#include <CZ/Louvre/Manager/LImageCaptureFrame.h>

using namespace CZ;
using namespace CZ::Protocols::ImageCopyCapture;

static const struct ext_image_copy_capture_frame_v1_interface imp
{
    .destroy = &RImageCopyCaptureFrame::destroy,
    .attach_buffer = &RImageCopyCaptureFrame::attach_buffer,
    .damage_buffer = &RImageCopyCaptureFrame::damage_buffer,
    .capture = &RImageCopyCaptureFrame::capture
};

RImageCopyCaptureFrame::RImageCopyCaptureFrame(RImageCopyCaptureSession &sessionRes, UInt32 id) noexcept
    :LResource
    (
        sessionRes.client(),
        &ext_image_copy_capture_frame_v1_interface,
        sessionRes.version(),
        id,
        &imp
    )
{
    if (sessionRes.session())
        m_frame.reset(new LImageCaptureFrame(*this, *sessionRes.session()));
}

RImageCopyCaptureFrame::~RImageCopyCaptureFrame() noexcept
{
    m_frame.reset();
}

/******************** REQUESTS ********************/

void RImageCopyCaptureFrame::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RImageCopyCaptureFrame::attach_buffer(wl_client */*client*/, wl_resource *resource, wl_resource *buffer) noexcept
{
    auto &res { LRES_CAST(RImageCopyCaptureFrame, resource) };

    if (res.m_captured)
    {
        res.postError(EXT_IMAGE_COPY_CAPTURE_FRAME_V1_ERROR_ALREADY_CAPTURED, "attach_buffer sent after capture.");
        return;
    }

    res.m_buffer.setResource(buffer);
}

void RImageCopyCaptureFrame::damage_buffer(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height) noexcept
{
    auto &res { LRES_CAST(RImageCopyCaptureFrame, resource) };

    if (res.m_captured)
    {
        res.postError(EXT_IMAGE_COPY_CAPTURE_FRAME_V1_ERROR_ALREADY_CAPTURED, "damage_buffer sent after capture.");
        return;
    }

    if (x < 0 || y < 0 || width <= 0 || height <= 0)
    {
        res.postError(EXT_IMAGE_COPY_CAPTURE_FRAME_V1_ERROR_INVALID_BUFFER_DAMAGE, "Invalid buffer damage ({}, {}, {}, {}).", x, y, width, height);
        return;
    }

    if (res.m_frame)
        res.m_frame->m_clientDamage.op(SkIRect::MakeXYWH(x, y, width, height), SkRegion::kUnion_Op);
}

void RImageCopyCaptureFrame::capture(wl_client */*client*/, wl_resource *resource) noexcept
{
    auto &res { LRES_CAST(RImageCopyCaptureFrame, resource) };

    if (res.m_captured)
    {
        res.postError(EXT_IMAGE_COPY_CAPTURE_FRAME_V1_ERROR_ALREADY_CAPTURED, "capture sent twice.");
        return;
    }

    if (!res.m_buffer.res())
    {
        res.postError(EXT_IMAGE_COPY_CAPTURE_FRAME_V1_ERROR_NO_BUFFER, "capture sent without attach_buffer.");
        return;
    }

    res.m_captured = true;

    // Its session is gone (or never existed)
    if (!res.m_frame)
    {
        res.failed(EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_STOPPED);
        return;
    }

    res.m_frame->m_captureRequested = true;
    res.m_frame->session().captureRequested();
}

/******************** EVENTS ********************/

void RImageCopyCaptureFrame::transform(CZTransform transform) noexcept
{
    ext_image_copy_capture_frame_v1_send_transform(resource(), static_cast<UInt32>(transform));
}

void RImageCopyCaptureFrame::damage(const SkIRect &rect) noexcept
{
    ext_image_copy_capture_frame_v1_send_damage(resource(), rect.x(), rect.y(), rect.width(), rect.height());
}

void RImageCopyCaptureFrame::presentationTime(const timespec &time) noexcept
{
    const UInt64 sec { static_cast<UInt64>(time.tv_sec) };
    ext_image_copy_capture_frame_v1_send_presentation_time(resource(), sec >> 32, sec & 0xffffffff, static_cast<UInt32>(time.tv_nsec));
}

void RImageCopyCaptureFrame::ready() noexcept
{
    ext_image_copy_capture_frame_v1_send_ready(resource());
}

void RImageCopyCaptureFrame::failed(UInt32 reason) noexcept
{
    ext_image_copy_capture_frame_v1_send_failed(resource(), reason);
}
