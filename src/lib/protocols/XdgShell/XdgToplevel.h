#ifndef XDGTOPLEVEL_H
#define XDGTOPLEVEL_H

#include <LNamespaces.h>

class Louvre::Extensions::XdgShell::Toplevel
{
public:
    static void destroy_resource(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void set_parent(wl_client *client, wl_resource *resource, wl_resource *parent);
    static void set_title(wl_client *client, wl_resource *resource, const char *title);
    static void set_app_id(wl_client *client, wl_resource *resource, const char *app_id);
    static void show_window_menu(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial, Int32 x, Int32 y);
    static void move(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial);
    static void resize(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial, UInt32 edges);
    static void set_max_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height);
    static void set_min_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height);
    static void set_maximized(wl_client *client, wl_resource *resource);
    static void unset_maximized(wl_client *client, wl_resource *resource);
    static void set_fullscreen(wl_client *client, wl_resource *resource, wl_resource *output);
    static void unset_fullscreen(wl_client *client, wl_resource *resource);
    static void set_minimized(wl_client *client, wl_resource *resource);
};

#endif // XDGTOPLEVEL_H
