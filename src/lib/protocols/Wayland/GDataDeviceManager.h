#ifndef DATADEVICEMANAGERGLOBAL_H
#define DATADEVICEMANAGERGLOBAL_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GDataDeviceManager : public LResource
{
public:
    GDataDeviceManager(LClient *client, const wl_interface *interface, Int32 version, UInt32 id, const void *implementation, wl_resource_destroy_func_t destroy);
    ~GDataDeviceManager();

    LPRIVATE_IMP(GDataDeviceManager)
};
#endif // DATADEVICEMANAGERGLOBAL_H
