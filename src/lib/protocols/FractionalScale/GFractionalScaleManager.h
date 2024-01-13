#ifndef GFRACTIONALSCALEMANAGER_H
#define GFRACTIONALSCALEMANAGER_H

#include <LResource.h>

class Louvre::Protocols::FractionalScale::GFractionalScaleManager : public LResource
{
public:
    GFractionalScaleManager(wl_client *client,
                 const wl_interface *interface,
                 Int32 version,
                 UInt32 id,
                 const void *implementation,
                 wl_resource_destroy_func_t destroy);

    ~GFractionalScaleManager();

    LPRIVATE_IMP_UNIQUE(GFractionalScaleManager)
};

#endif // GFRACTIONALSCALEMANAGER_H
