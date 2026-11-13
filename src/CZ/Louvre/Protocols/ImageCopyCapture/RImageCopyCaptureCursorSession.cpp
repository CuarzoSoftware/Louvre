#include <CZ/Louvre/Protocols/ImageCopyCapture/ext-image-copy-capture-v1.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureCursorSession.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureSession.h>
#include <CZ/Louvre/Manager/LImageCaptureManager.h>
#include <CZ/Louvre/Manager/LImageCaptureSession.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>

using namespace CZ;
using namespace CZ::Protocols::ImageCopyCapture;

static const struct ext_image_copy_capture_cursor_session_v1_interface imp
{
    .destroy = &RImageCopyCaptureCursorSession::destroy,
    .get_capture_session = &RImageCopyCaptureCursorSession::get_capture_session
};

RImageCopyCaptureCursorSession::RImageCopyCaptureCursorSession(LClient *client, Int32 version, UInt32 id, LImageCaptureSourceType type, LOutput *output, LToplevelRole *toplevel) noexcept
    :LResource
    (
        client,
        &ext_image_copy_capture_cursor_session_v1_interface,
        version,
        id,
        &imp
    ),
    m_type(type),
    m_output(output),
    m_toplevel(toplevel)
{}

/******************** REQUESTS ********************/

void RImageCopyCaptureCursorSession::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RImageCopyCaptureCursorSession::get_capture_session(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    auto &res { LRES_CAST(RImageCopyCaptureCursorSession, resource) };

    if (res.m_hasSession)
    {
        res.postError(EXT_IMAGE_COPY_CAPTURE_CURSOR_SESSION_V1_ERROR_DUPLICATE_SESSION, "get_capture_session sent twice.");
        return;
    }

    res.m_hasSession = true;
    auto *sessionRes { new RImageCopyCaptureSession(res.client(), res.version(), id) };

    // Nothing to capture anymore
    if (!res.m_output && !res.m_toplevel)
    {
        sessionRes->stopped();
        return;
    }

    sessionRes->m_session.reset(new LImageCaptureSession(*sessionRes, res.m_type, res.m_output, res.m_toplevel, false, &res));
    auto *session { sessionRes->m_session.get() };
    imageCaptureManager()->m_sessions.emplace_back(session);

    // Like any capture: the compositor decides (possibly later) whether to accept it
    imageCaptureManager()->sessionRequest(session);
}

/******************** EVENTS ********************/

void RImageCopyCaptureCursorSession::enter() noexcept
{
    ext_image_copy_capture_cursor_session_v1_send_enter(resource());
}

void RImageCopyCaptureCursorSession::leave() noexcept
{
    ext_image_copy_capture_cursor_session_v1_send_leave(resource());
}

void RImageCopyCaptureCursorSession::position(Int32 x, Int32 y) noexcept
{
    ext_image_copy_capture_cursor_session_v1_send_position(resource(), x, y);
}

void RImageCopyCaptureCursorSession::hotspot(Int32 x, Int32 y) noexcept
{
    ext_image_copy_capture_cursor_session_v1_send_hotspot(resource(), x, y);
}
