#ifndef GSEATPRIVATE_H
#define GSEATPRIVATE_H

#include <protocols/Wayland/GSeat.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(GSeat)
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
    static void resource_destroy(wl_resource *resource);
    static void get_pointer(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_keyboard(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_touch(wl_client *client, wl_resource *resource, UInt32 id);
#if LOUVRE_WL_SEAT_VERSION >= 5
    static void release(wl_client *client, wl_resource *resource);
#endif

    RKeyboard *keyboardResource = nullptr;
    RPointer *pointerResource = nullptr;
    RDataDevice *dataDeviceResource = nullptr;
    std::list<GSeat*>::iterator clientLink;
};

#endif // GSEATPRIVATE_H
