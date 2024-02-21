#ifndef GDATADEVICEMANAGER_H
#define GDATADEVICEMANAGER_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GDataDeviceManager : public LResource
{
public:
    GDataDeviceManager(LClient *client,
                       const wl_interface *interface,
                       Int32 version,
                       UInt32 id,
                       const void *implementation);
    ~GDataDeviceManager();

    LPRIVATE_IMP_UNIQUE(GDataDeviceManager)
};
#endif // GDATADEVICEMANAGER_H
