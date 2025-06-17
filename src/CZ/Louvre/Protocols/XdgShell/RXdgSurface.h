#ifndef RXDGSURFACE_H
#define RXDGSURFACE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/CZWeak.h>
#include <CZ/skia/core/SkRect.h>

class Louvre::Protocols::XdgShell::RXdgSurface final : public LResource
{
public:
    GXdgWmBase *xdgWmBaseRes() const noexcept
    {
        return m_xdgWmBaseRes;
    }

    LSurface *surface() const noexcept
    {
        return m_surface;
    }

    RXdgToplevel *xdgToplevelRes() const noexcept
    {
        return m_xdgToplevelRes;
    }

    RXdgPopup *xdgPopupRes() const noexcept
    {
        return m_xdgPopupRes;
    }

    const SkIRect &windowGeometry() const noexcept
    {
        return m_currentWindowGeometry;
    }

    SkIRect calculateGeometryWithSubsurfaces() noexcept;

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) ;
    static void get_toplevel(wl_client *client,wl_resource *resource, UInt32 id);
    static void get_popup(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *parent, wl_resource *positioner);
    static void set_window_geometry(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void ack_configure(wl_client *client, wl_resource *resource, UInt32 serial);

    /******************** EVENTS ********************/

    // Since 1
    void configure(UInt32 serial) noexcept;

private:
    friend class Louvre::Protocols::XdgShell::GXdgWmBase;
    friend class Louvre::Protocols::XdgShell::RXdgToplevel;
    friend class Louvre::Protocols::XdgShell::RXdgPopup;
    friend class Louvre::LToplevelRole;
    friend class Louvre::LPopupRole;

    RXdgSurface(GXdgWmBase *xdgWmBaseRes, LSurface *surface, UInt32 id) noexcept;
    ~RXdgSurface() noexcept;
    CZWeak<GXdgWmBase> m_xdgWmBaseRes;
    CZWeak<LSurface> m_surface;
    CZWeak<RXdgPopup> m_xdgPopupRes;
    CZWeak<RXdgToplevel> m_xdgToplevelRes;
    SkIRect m_currentWindowGeometry { 0, 0, 0, 0 };
    SkIRect m_pendingWindowGeometry { 0, 0, 0, 0 };
    bool m_windowGeometrySet { false };
    bool m_hasPendingWindowGeometry { false };
};

#endif // RXDGSURFACE_H
