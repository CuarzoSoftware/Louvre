#ifndef RSURFACEPRIVATE_H
#define RSURFACEPRIVATE_H

#include <protocols/Wayland/RSurface.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RSurface)
    static void resource_destroy(wl_resource *resource);
    static void attach(wl_client *client, wl_resource *resource, wl_resource *buffer, Int32 x, Int32 y);
    static void frame(wl_client *client, wl_resource *resource, UInt32 callback);
    static void destroy(wl_client *client, wl_resource *resource);
    static void commit(wl_client *client, wl_resource *resource);
    static void damage(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void set_opaque_region(wl_client *client, wl_resource *resource, wl_resource *region);
    static void set_input_region(wl_client *client, wl_resource *resource, wl_resource *region);

    #if LOUVRE_WL_COMPOSITOR_VERSION >= 2
    static void set_buffer_transform(wl_client *client, wl_resource *resource, Int32 transform);
    #endif

    #if LOUVRE_WL_COMPOSITOR_VERSION >= 3
    static void set_buffer_scale(wl_client *client, wl_resource *resource, Int32 scale);
    #endif

    #if LOUVRE_WL_COMPOSITOR_VERSION >= 4
    static void damage_buffer(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 w, Int32 h);
    #endif

    #if LOUVRE_WL_COMPOSITOR_VERSION >= 5
    static void offset(wl_client *client, wl_resource *resource, Int32 x, Int32 y);
    #endif

    static void handleOffset(LSurface *lSurface, Int32 x, Int32 y);
    static void apply_commit(LSurface *surface, CommitOrigin origin = Itself);

    LSurface *lSurface = nullptr;
};

#endif // RSURFACEPRIVATE_H
