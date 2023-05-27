#ifndef RXDGTOPLEVELDECORATIONPRIVATE_H
#define RXDGTOPLEVELDECORATIONPRIVATE_H

#include <protocols/XdgDecoration/RXdgToplevelDecoration.h>

using namespace Louvre::Protocols::XdgDecoration;

LPRIVATE_CLASS(RXdgToplevelDecoration)
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void set_mode(wl_client *client, wl_resource *resource, UInt32 mode);
    static void unset_mode(wl_client *client, wl_resource *resource);

    LToplevelRole *lToplevelRole = nullptr;
};

#endif // RXDGTOPLEVELDECORATIONPRIVATE_H
