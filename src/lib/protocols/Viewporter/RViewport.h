#ifndef RVIEWPORT_H
#define RVIEWPORT_H

#include <LResource.h>
#include <LRect.h>
#include <LWeak.h>

class Louvre::Protocols::Viewporter::RViewport final : public LResource
{
public:
    Wayland::RSurface *surfaceRes() const noexcept
    {
        return m_surfaceRes.get();
    }

    const LSize &dstSize() const noexcept
    {
        return m_dstSize;

    }
    const LRectF &srcRect() const noexcept
    {
        return m_srcRect;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void set_source(wl_client *client, wl_resource *resource, Float24 x, Float24 y, Float24 width, Float24 height) noexcept;
    static void set_destination(wl_client *client, wl_resource *resource, Int32 width, Int32 height) noexcept;

private:
    friend class Louvre::Protocols::Viewporter::GViewporter;
    RViewport(Wayland::RSurface *surfaceRes, Int32 version, UInt32 id) noexcept;
    ~RViewport() noexcept = default;
    LSize m_dstSize { -1, -1 };
    LRectF m_srcRect { -1, -1, -1, -1 };
    LWeak<Wayland::RSurface> m_surfaceRes;
};

#endif // RVIEWPORT_H
