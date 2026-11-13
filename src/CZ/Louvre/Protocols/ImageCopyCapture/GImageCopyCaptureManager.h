#ifndef GIMAGECOPYCAPTUREMANAGER_H
#define GIMAGECOPYCAPTUREMANAGER_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::ImageCopyCapture::GImageCopyCaptureManager final : public LResource
{
public:
    static void create_session(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *source, UInt32 options) noexcept;
    static void create_pointer_cursor_session(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *source, wl_resource *pointer) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;

private:
    LGLOBAL_INTERFACE
    GImageCopyCaptureManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GImageCopyCaptureManager() noexcept = default;
};

#endif // GIMAGECOPYCAPTUREMANAGER_H
