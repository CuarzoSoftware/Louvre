#ifndef RXDGSURFACE_H
#define RXDGSURFACE_H

#include <LResource.h>
#include <LWeak.h>
#include <LRect.h>

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

    const LRect &windowGeometry() const noexcept
    {
        return m_currentWindowGeometry;
    }

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
    LWeak<GXdgWmBase> m_xdgWmBaseRes;
    LWeak<LSurface> m_surface;
    LWeak<RXdgPopup> m_xdgPopupRes;
    LWeak<RXdgToplevel> m_xdgToplevelRes;
    LRect m_currentWindowGeometry;
    LRect m_pendingWindowGeometry;
    bool m_windowGeometrySet { false };
    bool m_hasPendingWindowGeometry { false };
};

#endif // RXDGSURFACE_H
