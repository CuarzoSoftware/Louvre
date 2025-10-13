#ifndef RVIEWPORT_H
#define RVIEWPORT_H

#include <CZ/Louvre/LResource.h>
#include <CZ/skia/core/SkRect.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::Viewporter::RViewport final : public LResource
{
public:
    Wayland::RWlSurface *surfaceRes() const noexcept
    {
        return m_surfaceRes;
    }

    const SkISize &dstSize() const noexcept
    {
        return m_dstSize;

    }
    const SkRect &srcRect() const noexcept
    {
        return m_srcRect;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void set_source(wl_client *client, wl_resource *resource, Float24 x, Float24 y, Float24 width, Float24 height) noexcept;
    static void set_destination(wl_client *client, wl_resource *resource, Int32 width, Int32 height) noexcept;

private:
    friend class CZ::Protocols::Viewporter::GViewporter;
    RViewport(Wayland::RWlSurface *surfaceRes, Int32 version, UInt32 id) noexcept;
    ~RViewport() noexcept = default;
    SkISize m_dstSize { -1, -1 };
    SkRect m_srcRect { SkRect::MakeXYWH(-1, -1, -1, -1) };
    CZWeak<Wayland::RWlSurface> m_surfaceRes;
};

#endif // RVIEWPORT_H
