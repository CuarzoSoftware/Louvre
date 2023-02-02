#ifndef DATAOFFERRESOURCEPRIVATE_H
#define DATAOFFERRESOURCEPRIVATE_H

#include <protocols/Wayland/DataOfferResource.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(DataOfferResource)

    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void accept(wl_client *client, wl_resource *resource, UInt32 serial, const char *mime_type);
    static void receive(wl_client *client, wl_resource *resource, const char *mime_type, Int32 fd);

    #if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    static void finish(wl_client *client, wl_resource *resource);
    static void set_actions(wl_client *client, wl_resource *resource, UInt32 dnd_actions, UInt32 preferred_action);
    #endif

    LDataOffer *dataOffer = nullptr;
    DataDeviceResource *dataDeviceResource = nullptr;
};

#endif // DATAOFFERRESOURCEPRIVATE_H
