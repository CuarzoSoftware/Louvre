#ifndef GDATADEVICEMANAGERPRIVATE_H
#define GDATADEVICEMANAGERPRIVATE_H

#include <protocols/Wayland/GDataDeviceManager.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(GDataDeviceManager)
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
    static void create_data_source(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_data_device(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *seat);
};

#endif // GDATADEVICEMANAGERPRIVATE_H
