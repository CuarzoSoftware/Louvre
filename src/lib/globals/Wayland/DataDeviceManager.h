#ifndef DATADEVICEMANAGER_H
#define DATADEVICEMANAGER_H

#include <LNamespaces.h>

class Louvre::Globals::DataDeviceManager
{
public:
    static void create_data_source(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_data_device(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *seat);
    static void resource_destroy(wl_resource *resource);
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
};
#endif // DATADEVICEMANAGER_H
