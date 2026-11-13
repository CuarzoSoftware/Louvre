#ifndef LIMAGECAPTURESESSION_H
#define LIMAGECAPTURESESSION_H

#include <CZ/Louvre/LObject.h>
#include <CZ/Ream/Ream.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/skia/core/SkSize.h>
#include <memory>

/**
 * @brief A client capturing an output or a toplevel
 *
 * Created when a client starts an [ext-image-copy-capture](https://wayland.app/protocols/ext-image-copy-capture-v1)
 * session. It starts pending: LImageCaptureManager::sessionRequest() decides, possibly later, whether
 * to accept() or stop() it. Once accepted, the client receives the buffer constraints (bufferSize(),
 * formats) and can request frames (see LImageCaptureFrame).
 *
 * Changes are tracked as damage, in buffer coordinates: each frame carries (and copies) only what
 * changed since the client's previous frame. The first frame is always complete.
 *
 * Cursor sessions (isCursorSession()) capture the cursor image instead, and the client gets the
 * cursor's position over the source separately (handled by Louvre).
 *
 * The session is destroyed by its client (see LImageCaptureManager::sessionDestroyed()). It's
 * stopped when its source (output or toplevel) is destroyed.
 */
class CZ::LImageCaptureSession final : public LObject
{
public:
    /// State of the session
    enum State
    {
        Pending,  ///< Waiting to be accepted or stopped (no frames are captured)
        Accepted, ///< Capturing
        Stopped   ///< Denied or ended, it won't capture anymore
    };

    /// The capturing client.
    LClient *client() const noexcept;

    /// What is captured.
    LImageCaptureSourceType sourceType() const noexcept { return m_sourceType; }

    /// The captured output (Output sources, `nullptr` once destroyed).
    LOutput *output() const noexcept { return m_output.get(); }

    /// The captured toplevel (ForeignToplevel sources, `nullptr` once destroyed).
    LToplevelRole *toplevel() const noexcept { return m_toplevel.get(); }

    /**
     * @brief Whether the cursor is painted onto the frames.
     *
     * While such a session is accepted, the cursor of its output is painted in software (see
     * LCursor::isPlaneEnabled()) so it's part of the frames and of their damage.
     */
    bool paintsCursors() const noexcept { return m_paintsCursors; }

    /**
     * @brief Whether it captures the cursor image (a cursor session).
     *
     * Its frames are the cursor image, at the scale of the source (its buffer size follows the cursor
     * size), sent again when the image changes. Louvre also tells the client where the cursor is over
     * the source (enter, leave, position and hotspot), while accepted.
     */
    bool isCursorSession() const noexcept { return m_isCursor; }

    State state() const noexcept { return m_state; }

    /**
     * @brief Accepts a pending session.
     *
     * Sends the buffer constraints to the client, which can then capture frames. No-op if not pending.
     */
    void accept() noexcept;

    /**
     * @brief Denies a pending session or ends an accepted one.
     *
     * The client is notified (`stopped`) and its pending frame, if any, fails. It can't be undone.
     */
    void stop() noexcept;

    /**
     * @brief Size of the frames in buffer pixels.
     *
     * The output's mode size (output sources), or the toplevel's window geometry times its buffer
     * scale (toplevel sources). See setBufferSize().
     */
    SkISize bufferSize() const noexcept { return m_bufferSize; }

    /**
     * @brief Changes the size of the frames.
     *
     * Useful when rendering toplevel frames yourself (see LToplevelRole::imageCaptureFrameRequest())
     * with a different size, e.g. including server-side decorations. The client receives the new
     * constraints, the current frame fails (it must reallocate its buffers) and the next one is complete.
     */
    void setBufferSize(SkISize size) noexcept;

    /**
     * @brief Marks a region as changed, in buffer coordinates.
     *
     * Louvre tracks the damage of outputs and toplevel surfaces itself. Use this for changes it
     * can't know about, e.g. the server-side decorations of a captured toplevel. A frame waiting for
     * changes is then captured.
     */
    void addDamage(const SkRegion &region) noexcept;

    /// The changes not yet sent to the client (buffer coordinates).
    const SkRegion &damage() const noexcept { return m_damage; }

    /// The current frame, if any (at most one at a time).
    LImageCaptureFrame *frame() const noexcept { return m_frame; }

    ~LImageCaptureSession() noexcept;

private:
    friend class LImageCaptureFrame;
    friend class Protocols::ImageCopyCapture::GImageCopyCaptureManager;
    friend class Protocols::ImageCopyCapture::RImageCopyCaptureSession;
    friend class Protocols::ImageCopyCapture::RImageCopyCaptureFrame;
    friend class LOutput;
    friend class LSurface;

    LImageCaptureSession(Protocols::ImageCopyCapture::RImageCopyCaptureSession &res, LImageCaptureSourceType type, LOutput *output, LToplevelRole *toplevel, bool paintsCursors,
                         Protocols::ImageCopyCapture::RImageCopyCaptureCursorSession *cursorRes) noexcept;

    // Cursor sessions
    friend class LCursor;
    friend class Protocols::ImageCopyCapture::RImageCopyCaptureCursorSession;
    static void CursorChanged(bool imageChanged) noexcept; // From LCursor::update()
    void updateCursor(bool imageChanged) noexcept;
    SkISize cursorBufferSize() const noexcept;
    Float32 sourceScale() const noexcept;
    void renderCursorFrame() noexcept;
    CZWeak<Protocols::ImageCopyCapture::RImageCopyCaptureCursorSession> m_cursorRes;
    bool m_isCursor { false };
    bool m_cursorInside { false };
    SkIPoint m_cursorPos { INT32_MIN, INT32_MIN }, m_cursorHotspot { INT32_MIN, INT32_MIN };

    void sendConstraints() noexcept;

    // A frame was asked to be captured (or the session was just accepted with one waiting)
    void captureRequested() noexcept;

    // Toplevel sources: the window's size or scale may have changed (resends the constraints)
    void updateToplevelBufferSize() noexcept;

    // Output sources: copies the output image into the frame, if it's waiting and has changes
    void captureOutput(bool withCursor) noexcept;

    // Starts/ends forcing the software cursor on the output (paint cursors sessions)
    void setForcingSoftwareCursor(bool forcing) noexcept;

    // A surface was committed: damage of the captured toplevels it belongs to
    static void SurfaceCommitted(LSurface &surface) noexcept;

    Protocols::ImageCopyCapture::RImageCopyCaptureSession &m_res;
    LImageCaptureSourceType m_sourceType;
    CZWeak<LOutput> m_output;
    CZWeak<LToplevelRole> m_toplevel;
    bool m_paintsCursors;
    bool m_forcingSoftwareCursor { false };
    State m_state { Pending };
    SkISize m_bufferSize {};
    bool m_customBufferSize { false }; // Set with setBufferSize() (toplevels)
    SkRegion m_damage;
    bool m_firstFrame { true };
    LImageCaptureFrame *m_frame { nullptr };

    // Toplevel frames into shm buffers are rendered here, then copied (it keeps the full content)
    std::shared_ptr<RSurface> m_staging;
    bool m_stagingFresh { true };
};

#endif // LIMAGECAPTURESESSION_H
