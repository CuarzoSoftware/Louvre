#ifndef RXDGSURFACE_H
#define RXDGSURFACE_H

#include <protocols/XdgShell/RXdgSurface.h>

using namespace Louvre::Protocols::XdgShell;
using namespace std;

LPRIVATE_CLASS(RXdgSurface)
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_toplevel(wl_client *client,wl_resource *resource, UInt32 id);
    static void get_popup(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *parent, wl_resource *positioner);
    static void set_window_geometry(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void ack_configure(wl_client *client, wl_resource *resource, UInt32 serial);

    GXdgWmBase *gXdgWmBase = nullptr;
    LSurface *lSurface = nullptr;
    list<RXdgSurface*>::iterator xdgWmBaseLink;

    RXdgPopup *rXdgPopup = nullptr;
    RXdgToplevel *rXdgToplevel = nullptr;
};

#endif // RXDGSURFACE_H
