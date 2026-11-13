#include <CZ/Louvre/Protocols/ImageCopyCapture/ext-image-copy-capture-v1.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureSession.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureFrame.h>
#include <CZ/Louvre/Manager/LImageCaptureSession.h>
#include <CZ/Louvre/Manager/LImageCaptureFrame.h>

using namespace CZ;
using namespace CZ::Protocols::ImageCopyCapture;

static const struct ext_image_copy_capture_session_v1_interface imp
{
    .create_frame = &RImageCopyCaptureSession::create_frame,
    .destroy = &RImageCopyCaptureSession::destroy
};

RImageCopyCaptureSession::RImageCopyCaptureSession(LClient *client, Int32 version, UInt32 id) noexcept
    :LResource
    (
        client,
        &ext_image_copy_capture_session_v1_interface,
        version,
        id,
        &imp
    )
{}

RImageCopyCaptureSession::~RImageCopyCaptureSession() noexcept
{
    // Frames outlive their session (they become inert, see RImageCopyCaptureFrame)
    if (m_frameRes && m_frameRes->m_frame)
    {
        if (m_frameRes->m_captured && !m_frameRes->m_frame->done())
            m_frameRes->m_frame->fail(LImageCaptureFrame::Stopped);

        m_frameRes->m_frame.reset();
    }

    m_session.reset();
}

/******************** REQUESTS ********************/

void RImageCopyCaptureSession::create_frame(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    auto &res { LRES_CAST(RImageCopyCaptureSession, resource) };

    if (res.m_frameRes)
    {
        res.postError(EXT_IMAGE_COPY_CAPTURE_SESSION_V1_ERROR_DUPLICATE_FRAME, "create_frame sent before destroying the previous frame.");
        return;
    }

    res.m_frameRes.reset(new RImageCopyCaptureFrame(res, id));
}

void RImageCopyCaptureSession::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RImageCopyCaptureSession::stopped() noexcept
{
    ext_image_copy_capture_session_v1_send_stopped(resource());
}
