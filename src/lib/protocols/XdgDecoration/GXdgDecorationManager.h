#ifndef XDGDECORATIONMANAGER_H
#define XDGDECORATIONMANAGER_H

#include <LResource.h>

class Louvre::Protocols::XdgDecoration::GXdgDecorationManager : public LResource
{
public:
    GXdgDecorationManager(LCompositor *compositor,
                          wl_client *client,
                          const wl_interface *interface,
                          Int32 version,
                          UInt32 id,
                          const void *implementation,
                          wl_resource_destroy_func_t destroy);
    ~GXdgDecorationManager();


    LPRIVATE_IMP(GXdgDecorationManager)
};

#endif // XDGDECORATIONMANAGER_H
