#ifndef RDATASOURCEPRIVATE_H
#define RDATASOURCEPRIVATE_H

#include <protocols/Wayland/RDataSource.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RDataSource)
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void offer(wl_client *client, wl_resource *resource, const char *mime_type);

    #if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    static void set_actions(wl_client *client, wl_resource *resource, UInt32 dnd_actions);
    #endif

    LDataSource *lDataSource = nullptr;
};

#endif // RDATASOURCEPRIVATE_H
