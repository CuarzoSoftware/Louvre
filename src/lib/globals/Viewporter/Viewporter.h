#ifdef LOUVRE_VIEWPORTER_ENABLED
#ifndef VIEWPORTER_H
#define VIEWPORTER_H

#include <LNamespaces.h>

class Louvre::Extensions::Viewporter::Viewporter
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_viewport(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
};

#endif // VIEWPORTER_H
#endif
