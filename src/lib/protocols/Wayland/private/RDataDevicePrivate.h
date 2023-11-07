#ifndef RDATADEVICEPRIVATE_H
#define RDATADEVICEPRIVATE_H

#include <protocols/Wayland/RDataDevice.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RDataDevice)
    static void resource_destroy(wl_resource *resource);
    static void start_drag(wl_client *client, wl_resource *resource, wl_resource *source, wl_resource *origin, wl_resource *icon, UInt32 serial);
    static void set_selection(wl_client *client, wl_resource *resource, wl_resource *source, UInt32 serial);

    #if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 2
    static void release(wl_client *client, wl_resource *resource);
    #endif

    GSeat *gSeat = nullptr;
    LDataOffer *dataOffered = nullptr;
    LastEventSerials serials;
};

#endif // RDATADEVICEPRIVATE_H
