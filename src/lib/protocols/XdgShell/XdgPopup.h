#ifndef XDGPOPUP_H
#define XDGPOPUP_H

#include <LNamespaces.h>

class Louvre::Extensions::XdgShell::Popup
{
public:
    static void destroy_resource(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void grab(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial);

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    static void reposition(wl_client *client, wl_resource *resource, wl_resource *positioner, UInt32 token);
#endif

};
#endif // XDGPOPUP_H
