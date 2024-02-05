#ifndef GTEARINGCONTROLMANAGER_H
#define GTEARINGCONTROLMANAGER_H

#include <LResource.h>

class Louvre::Protocols::TearingControl::GTearingControlManager : public LResource
{
public:
    GTearingControlManager(LClient *client,
                         const wl_interface *interface,
                         Int32 version,
                         UInt32 id,
                         const void *implementation,
                         wl_resource_destroy_func_t destroy);
    ~GTearingControlManager();

    LPRIVATE_IMP_UNIQUE(GTearingControlManager)
};

#endif // GTEARINGCONTROLMANAGER_H
