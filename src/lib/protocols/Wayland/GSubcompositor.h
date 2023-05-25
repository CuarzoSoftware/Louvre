#ifndef SUBCOMPOSITOR_H
#define SUBCOMPOSITOR_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GSubcompositor : public LResource
{
public:
    GSubcompositor(LCompositor *compositor,
                   wl_client *client,
                   const wl_interface *interface,
                   Int32 version,
                   UInt32 id,
                   const void *implementation,
                   wl_resource_destroy_func_t destroy);

    ~GSubcompositor();

    LPRIVATE_IMP(GSubcompositor)
};

#endif // SUBCOMPOSITOR_H
