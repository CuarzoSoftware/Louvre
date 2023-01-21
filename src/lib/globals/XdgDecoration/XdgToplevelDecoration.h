#ifndef XDGTOPLEVELDECORATION_H
#define XDGTOPLEVELDECORATION_H

#include <LNamespaces.h>

class Louvre::Extensions::XdgDecoration::ToplevelDecoration
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void set_mode(wl_client *client, wl_resource *resource, UInt32 mode);
    static void unset_mode(wl_client *client, wl_resource *resource);
};


#endif // XDGTOPLEVELDECORATION_H
