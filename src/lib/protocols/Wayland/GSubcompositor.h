#ifndef GSUBCOMPOSITOR_H
#define GSUBCOMPOSITOR_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GSubcompositor : public LResource
{
public:
    GSubcompositor(wl_client *client,
                   const wl_interface *interface,
                   Int32 version,
                   UInt32 id,
                   const void *implementation,
                   wl_resource_destroy_func_t destroy);

    ~GSubcompositor();

    LPRIVATE_IMP_UNIQUE(GSubcompositor)
};

#endif // GSUBCOMPOSITOR_H
