#ifndef DATADEVICEMANAGERGLOBALPRIVATE_H
#define DATADEVICEMANAGERGLOBALPRIVATE_H

#include <protocols/Wayland/DataDeviceManagerGlobal.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(DataDeviceManagerGlobal)

    static void resource_destroy(wl_resource *resource);
    static void create_data_source(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_data_device(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *seat);
};

#endif // DATADEVICEMANAGERGLOBALPRIVATE_H
