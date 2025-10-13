#ifndef GBACKGROUNDBLURMANAGER_H
#define GBACKGROUNDBLURMANAGER_H

#include <CZ/Louvre/Roles/LBackgroundBlur.h>
#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZBitset.h>

class CZ::Protocols::BackgroundBlur::GBackgroundBlurManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_background_blur(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);

private:
    LGLOBAL_INTERFACE
    GBackgroundBlurManager(wl_client *client, Int32 version, UInt32 id);
    ~GBackgroundBlurManager() noexcept;

    CZBitset<LBackgroundBlur::MaskingCapabilities> m_maskingCapabilities;
};

#endif // GBACKGROUNDBLURMANAGER_H
