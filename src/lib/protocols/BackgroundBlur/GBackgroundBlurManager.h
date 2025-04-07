#ifndef GBACKGROUNDBLURMANAGER_H
#define GBACKGROUNDBLURMANAGER_H

#include <LResource.h>

class Louvre::Protocols::BackgroundBlur::GBackgroundBlurManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_background_blur(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);

private:
    LGLOBAL_INTERFACE
    GBackgroundBlurManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GBackgroundBlurManager() noexcept;
};

#endif // GBACKGROUNDBLURMANAGER_H
