#ifndef GXDGDECORATIONMANAGER_H
#define GXDGDECORATIONMANAGER_H

#include <LResource.h>

class Louvre::Protocols::XdgDecoration::GXdgDecorationManager : public LResource
{
public:
    GXdgDecorationManager(wl_client *client,
                          const wl_interface *interface,
                          Int32 version,
                          UInt32 id,
                          const void *implementation,
                          wl_resource_destroy_func_t destroy);

    ~GXdgDecorationManager();

    LPRIVATE_IMP_UNIQUE(GXdgDecorationManager)
};

#endif // GXDGDECORATIONMANAGER_H
