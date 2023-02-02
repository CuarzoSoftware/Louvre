#ifndef DATADEVICERESOURCEPRIVATE_H
#define DATADEVICERESOURCEPRIVATE_H

#include <protocols/Wayland/DataDeviceResource.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(DataDeviceResource)

    static void resource_destroy(wl_resource *resource);
    static void start_drag(wl_client *client, wl_resource *resource, wl_resource *source, wl_resource *origin, wl_resource *icon, UInt32 serial);
    static void set_selection(wl_client *client, wl_resource *resource, wl_resource *source, UInt32 serial);

    #if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= WL_DATA_DEVICE_RELEASE_SINCE_VERSION
    static void release(wl_client *client, wl_resource *resource);
    #endif

    SeatGlobal *seatGlobal = nullptr;
    LDataOffer *dataOffered = nullptr;
};

#endif // DATADEVICERESOURCEPRIVATE_H
