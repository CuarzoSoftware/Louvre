#ifndef RXDGPOSITIONERPRIVATE_H
#define RXDGPOSITIONERPRIVATE_H

#include <protocols/XdgShell/RXdgPositioner.h>
#include <LPositioner.h>

using namespace Louvre::Protocols::XdgShell;

LPRIVATE_CLASS(RXdgPositioner)
    static void destroy(wl_client *client, wl_resource *resource);
    static void set_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height);
    static void set_anchor_rect(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void set_anchor(wl_client *client, wl_resource *resource, UInt32 anchor);
    static void set_gravity(wl_client *client, wl_resource *resource, UInt32 gravity);
    static void set_constraint_adjustment(wl_client *client, wl_resource *resource, UInt32 constraintAdjustment);
    static void set_offset(wl_client *client, wl_resource *resource, Int32 x, Int32 y);

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    static void set_reactive(wl_client *client, wl_resource *resource);
    static void set_parent_size(wl_client *client, wl_resource *resource, Int32 parent_width, Int32 parent_height);
    static void set_parent_configure(wl_client *client, wl_resource *resource, UInt32 serial);
#endif

    LPositioner lPositioner;
};

#endif // RXDGPOSITIONERPRIVATE_H
