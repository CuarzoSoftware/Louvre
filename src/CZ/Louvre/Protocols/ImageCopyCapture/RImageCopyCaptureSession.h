#ifndef RIMAGECOPYCAPTURESESSION_H
#define RIMAGECOPYCAPTURESESSION_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>
#include <memory>

/**
 * @brief ext_image_copy_capture_session_v1
 *
 * Owns its LImageCaptureSession. Sessions without one (cursor sessions, sources already gone) are
 * stopped right away.
 */
class CZ::Protocols::ImageCopyCapture::RImageCopyCaptureSession final : public LResource
{
public:
    /// The session (nullptr for inert ones).
    LImageCaptureSession *session() const noexcept { return m_session.get(); }

    /// The frame resource currently alive, if any.
    RImageCopyCaptureFrame *frameRes() const noexcept { return m_frameRes.get(); }

    static void create_frame(wl_client *client, wl_resource *resource, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;

    void stopped() noexcept;

private:
    friend class GImageCopyCaptureManager;
    friend class RImageCopyCaptureCursorSession;
    friend class RImageCopyCaptureFrame;
    friend class CZ::LImageCaptureSession;
    friend class CZ::Protocols::ImageCopyCapture::RImageCopyCaptureCursorSession;
    RImageCopyCaptureSession(LClient *client, Int32 version, UInt32 id) noexcept;
    ~RImageCopyCaptureSession() noexcept;
    std::unique_ptr<LImageCaptureSession> m_session;
    CZWeak<RImageCopyCaptureFrame> m_frameRes;
};

#endif // RIMAGECOPYCAPTURESESSION_H
