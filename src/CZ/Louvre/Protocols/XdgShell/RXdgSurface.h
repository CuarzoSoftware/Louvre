#ifndef RXDGSURFACE_H
#define RXDGSURFACE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/skia/core/SkRect.h>

class CZ::Protocols::XdgShell::RXdgSurface final : public LResource
{
public:
    GXdgWmBase *xdgWmBaseRes() const noexcept { return m_xdgWmBaseRes; }
    LSurface *surface() const noexcept { return m_surface; }
    RXdgToplevel *xdgToplevelRes() const noexcept { return m_xdgToplevelRes; }
    RXdgPopup *xdgPopupRes() const noexcept { return m_xdgPopupRes; }

    static SkIRect CalculateGeometryWithSubsurfaces(LSurface *surface) noexcept;

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
    friend class CZ::Protocols::XdgShell::GXdgWmBase;
    friend class CZ::Protocols::XdgShell::RXdgToplevel;
    friend class CZ::Protocols::XdgShell::RXdgPopup;
    friend class CZ::LToplevelRole;
    friend class CZ::LPopupRole;

    RXdgSurface(GXdgWmBase *xdgWmBaseRes, LSurface *surface, UInt32 id) noexcept;
    ~RXdgSurface() noexcept;
    CZWeak<GXdgWmBase> m_xdgWmBaseRes;
    CZWeak<LSurface> m_surface;
    CZWeak<RXdgPopup> m_xdgPopupRes;
    CZWeak<RXdgToplevel> m_xdgToplevelRes;

    SkIRect m_windowGeometry {};
    bool m_windowGeometrySet {};
};

#endif // RXDGSURFACE_H
