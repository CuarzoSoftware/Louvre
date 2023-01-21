#ifndef REGION_H
#define REGION_H

#include <LNamespaces.h>

class Louvre::Globals::Region
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void add(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void subtract(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
};


#endif // REGION_H
