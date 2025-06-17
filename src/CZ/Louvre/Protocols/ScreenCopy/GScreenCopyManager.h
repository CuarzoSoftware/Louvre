#ifndef GSCREENCOPYMANAGER_H
#define GSCREENCOPYMANAGER_H

#include <CZ/Louvre/LResource.h>
#include <CZ/skia/core/SkRegion.h>
#include <map>

class Louvre::Protocols::ScreenCopy::GScreenCopyManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void capture_output(wl_client *client, wl_resource *resource, UInt32 id, Int32 overlayCursor, wl_resource *output) noexcept;
    static void capture_output_region(wl_client *client, wl_resource *resource, UInt32 id, Int32 overlayCursor, wl_resource *output,
        Int32 x, Int32 y, Int32 width, Int32 height) noexcept;

    struct OutputDamage
    {
        SkRegion damage;
        bool firstFrame { true };
    };

    std::map<LOutput *, OutputDamage> damage;
private:
    LGLOBAL_INTERFACE
    GScreenCopyManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GScreenCopyManager() noexcept;
};

#endif // GSCREENCOPYMANAGER_H
