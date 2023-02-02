#ifndef DATADEVICEMANAGERGLOBAL_H
#define DATADEVICEMANAGERGLOBAL_H

#include <LResource.h>

class Louvre::Protocols::Wayland::DataDeviceManagerGlobal : public LResource
{
public:
    DataDeviceManagerGlobal(LClient *client, const wl_interface *interface, Int32 version, UInt32 id, const void *implementation, wl_resource_destroy_func_t destroy);
    ~DataDeviceManagerGlobal();

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);

    LPRIVATE_IMP(DataDeviceManagerGlobal)
};
#endif // DATADEVICEMANAGERGLOBAL_H
