#ifndef RXDGPOPUPPRIVATE_H
#define RXDGPOPUPPRIVATE_H

#include <protocols/XdgShell/RXdgPopup.h>

using namespace Louvre::Protocols::XdgShell;

LPRIVATE_CLASS(RXdgPopup)
    static void destroy(wl_client *client, wl_resource *resource);
    static void grab(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial);

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    static void reposition(wl_client *client, wl_resource *resource, wl_resource *positioner, UInt32 token);
#endif

    RXdgSurface *rXdgSurface { nullptr };
    LPopupRole *lPopupRole { nullptr };
};
#endif // RXDGPOPUPPRIVATE_H
