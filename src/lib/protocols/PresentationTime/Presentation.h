#ifndef PRESENTATION_H
#define PRESENTATION_H

#include <LNamespaces.h>

class Louvre::Extensions::PresentationTime::Presentation
{
public:
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void feedback(wl_client *client, wl_resource *resource, wl_resource *surface, UInt32 id);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
};

#endif // PRESENTATION_H
