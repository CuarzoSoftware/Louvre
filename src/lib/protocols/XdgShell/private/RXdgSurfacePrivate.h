#ifndef RXDGSURFACEPRIVATE_H
#define RXDGSURFACEPRIVATE_H

#include <protocols/XdgShell/RXdgSurface.h>
#include <LRect.h>

using namespace Louvre::Protocols::XdgShell;

LPRIVATE_CLASS(RXdgSurface)
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_toplevel(wl_client *client,wl_resource *resource, UInt32 id);
    static void get_popup(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *parent, wl_resource *positioner);
    static void set_window_geometry(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void ack_configure(wl_client *client, wl_resource *resource, UInt32 serial);

    GXdgWmBase *gXdgWmBase { nullptr };
    LSurface *lSurface { nullptr };
    std::list<RXdgSurface*>::iterator xdgWmBaseLink;

    bool windowGeometrySet { false };
    LRect pendingWindowGeometry;
    LRect currentWindowGeometry;
    bool hasPendingWindowGeometry { false };

    RXdgPopup *rXdgPopup { nullptr };
    RXdgToplevel *rXdgToplevel { nullptr };
};

#endif // RXDGSURFACEPRIVATE_H
