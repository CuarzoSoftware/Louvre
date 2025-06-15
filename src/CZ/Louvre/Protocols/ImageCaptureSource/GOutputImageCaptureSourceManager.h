#ifndef GOUTPUTIMAGECAPTURESOURCEMANAGER_H
#define GOUTPUTIMAGECAPTURESOURCEMANAGER_H

#include <LResource.h>

class Louvre::Protocols::ImageCaptureSource::GOutputImageCaptureSourceManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void create_source(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *output) noexcept;
private:
    LGLOBAL_INTERFACE
    GOutputImageCaptureSourceManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GOutputImageCaptureSourceManager() noexcept;
};

#endif // GOUTPUTIMAGECAPTURESOURCEMANAGER_H
