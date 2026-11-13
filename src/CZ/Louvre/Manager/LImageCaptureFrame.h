#ifndef LIMAGECAPTUREFRAME_H
#define LIMAGECAPTUREFRAME_H

#include <CZ/Louvre/LObject.h>
#include <CZ/Ream/Ream.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/Core/CZTransform.h>
#include <memory>

/**
 * @brief A frame requested by an image capture session
 *
 * The client attaches a buffer (shm or dma-buf) of LImageCaptureSession::bufferSize(), tells what
 * it already has (clientDamage()) and asks to capture. Louvre fills it once there's something new
 * (the first frame of a session immediately), copying only what changed, and sends `ready`.
 *
 * Output frames are handled entirely by Louvre. Toplevel frames are rendered by
 * LToplevelRole::imageCaptureFrameRequest(): to render them yourself, draw paintRegion() into
 * surface() and call ready(), or call fail().
 */
class CZ::LImageCaptureFrame final : public LObject
{
public:
    /// Why a frame failed
    enum FailReason
    {
        Unknown           = 0, ///< A runtime error (the client may retry)
        BufferConstraints = 1, ///< The buffer doesn't match the session's constraints
        Stopped           = 2  ///< The session stopped
    };

    /// The session it belongs to.
    LImageCaptureSession &session() const noexcept { return m_session; }

    /**
     * @brief What the client asks to be updated, in buffer coordinates.
     *
     * The client's buffers may be stale there (e.g. reused buffers), in addition to the session's
     * damage. Louvre already considers it in paintRegion() and when copying.
     */
    const SkRegion &clientDamage() const noexcept { return m_clientDamage; }

    /**
     * @brief Where to render the frame (toplevel sources).
     *
     * Its geometry maps buffer pixels 1:1 (viewport and destination are the buffer rect). It's the
     * client's own buffer for dma-buf buffers, or an image whose content persists for shm buffers.
     *
     * @return `nullptr` on failure (then call fail()).
     */
    std::shared_ptr<RSurface> surface() noexcept;

    /**
     * @brief What must be rendered into surface(), in buffer coordinates.
     *
     * Everything for the first frame (or a new surface()), otherwise the session's damage (plus the
     * client damage when surface() is the client's buffer).
     */
    SkRegion paintRegion() noexcept;

    /**
     * @brief Completes the frame.
     *
     * Copies what's needed into the client's buffer (shm), sends the damage (the session's, or
     * everything for the first frame), @p transform and the presentation time, then `ready`.
     */
    void ready(CZTransform transform = CZTransform::Normal) noexcept;

    /// Fails the frame (`failed`).
    void fail(FailReason reason) noexcept;

    /// Whether ready() or fail() was already called.
    bool done() const noexcept { return m_done; }

    ~LImageCaptureFrame() noexcept;

private:
    friend class LImageCaptureSession;
    friend class Protocols::ImageCopyCapture::RImageCopyCaptureFrame;

    LImageCaptureFrame(Protocols::ImageCopyCapture::RImageCopyCaptureFrame &res, LImageCaptureSession &session) noexcept;

    // The attached buffer matches the constraints
    bool validateBuffer() noexcept;

    // The client's dma-buf image, if the buffer is a dma-buf
    std::shared_ptr<RImage> dmaImage() const noexcept;

    // Copies @p region of @p src (buffer sized, same orientation) into the client's buffer
    bool copyFrom(std::shared_ptr<RImage> src, const SkRegion &region) noexcept;

    void sendReady(const SkRegion &damage, CZTransform transform) noexcept;

    Protocols::ImageCopyCapture::RImageCopyCaptureFrame &m_res;
    LImageCaptureSession &m_session;
    SkRegion m_clientDamage;
    std::shared_ptr<RSurface> m_surface;
    bool m_captureRequested { false };
    bool m_done { false };
};

#endif // LIMAGECAPTUREFRAME_H
