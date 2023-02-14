#ifndef COMPOSITORGLOBAL_H
#define COMPOSITORGLOBAL_H

#include <LResource.h>

class Louvre::Protocols::Wayland::CompositorGlobal : public LResource
{
public:
    CompositorGlobal(LClient *client, const wl_interface *interface, Int32 version, UInt32 id, const void *implementation, wl_resource_destroy_func_t destroy);
    ~CompositorGlobal();

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);

    LPRIVATE_IMP(CompositorGlobal)
};


#endif // COMPOSITORGLOBAL_H
