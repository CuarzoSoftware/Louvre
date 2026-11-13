#ifndef RIMAGECOPYCAPTUREFRAME_H
#define RIMAGECOPYCAPTUREFRAME_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Louvre/Private/LResourceRef.h>
#include <CZ/Core/CZTransform.h>
#include <CZ/skia/core/SkRect.h>
#include <ctime>
#include <memory>

/**
 * @brief ext_image_copy_capture_frame_v1
 *
 * Owns its LImageCaptureFrame while its session exists (frames outlive destroyed sessions: they
 * become inert and fail).
 */
class CZ::Protocols::ImageCopyCapture::RImageCopyCaptureFrame final : public LResource
{
public:
    /// The frame (nullptr once its session is gone).
    LImageCaptureFrame *frame() const noexcept { return m_frame.get(); }

    /// The attached wl_buffer, if any (and still alive).
    wl_resource *buffer() const noexcept { return m_buffer.res(); }

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void attach_buffer(wl_client *client, wl_resource *resource, wl_resource *buffer) noexcept;
    static void damage_buffer(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height) noexcept;
    static void capture(wl_client *client, wl_resource *resource) noexcept;

    void transform(CZTransform transform) noexcept;
    void damage(const SkIRect &rect) noexcept;
    void presentationTime(const timespec &time) noexcept;
    void ready() noexcept;
    void failed(UInt32 reason) noexcept;

private:
    friend class RImageCopyCaptureSession;
    friend class CZ::LImageCaptureSession;
    RImageCopyCaptureFrame(RImageCopyCaptureSession &sessionRes, UInt32 id) noexcept;
    ~RImageCopyCaptureFrame() noexcept;
    std::unique_ptr<LImageCaptureFrame> m_frame;
    LResourceRef m_buffer;
    bool m_captured { false };
};

#endif // RIMAGECOPYCAPTUREFRAME_H
