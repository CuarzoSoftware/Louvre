#ifndef COMPOSITORGLOBAL_H
#define COMPOSITORGLOBAL_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GCompositor : public LResource
{
public:
    GCompositor(LClient *client, const wl_interface *interface, Int32 version, UInt32 id, const void *implementation, wl_resource_destroy_func_t destroy);
    ~GCompositor();

    LPRIVATE_IMP(GCompositor)
};

#endif // COMPOSITORGLOBAL_H
