#ifndef RIMAGECOPYCAPTURECURSORSESSION_H
#define RIMAGECOPYCAPTURECURSORSESSION_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

/**
 * @brief ext_image_copy_capture_cursor_session_v1
 *
 * The cursor over an output or toplevel. Its capture session (get_capture_session) is an
 * LImageCaptureSession with LImageCaptureSession::isCursorSession(), which drives these events.
 */
class CZ::Protocols::ImageCopyCapture::RImageCopyCaptureCursorSession final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void get_capture_session(wl_client *client, wl_resource *resource, UInt32 id) noexcept;

    void enter() noexcept;
    void leave() noexcept;
    void position(Int32 x, Int32 y) noexcept;
    void hotspot(Int32 x, Int32 y) noexcept;

private:
    friend class GImageCopyCaptureManager;
    RImageCopyCaptureCursorSession(LClient *client, Int32 version, UInt32 id, LImageCaptureSourceType type, LOutput *output, LToplevelRole *toplevel) noexcept;
    ~RImageCopyCaptureCursorSession() noexcept = default;
    LImageCaptureSourceType m_type;
    CZWeak<LOutput> m_output;
    CZWeak<LToplevelRole> m_toplevel;
    bool m_hasSession { false };
};

#endif // RIMAGECOPYCAPTURECURSORSESSION_H
