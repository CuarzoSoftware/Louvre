#ifndef SEATGLOBALPRIVATE_H
#define SEATGLOBALPRIVATE_H

#include <protocols/Wayland/GSeat.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(GSeat)
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
    static void resource_destroy(wl_resource *resource);
    static void get_pointer(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_keyboard(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_touch(wl_client *client, wl_resource *resource, UInt32 id);
#if LOUVRE_SEAT_VERSION >= WL_SEAT_RELEASE_SINCE_VERSION
    static void release(wl_client *client, wl_resource *resource);
#endif

    RKeyboard *keyboardResource = nullptr;
    RPointer *pointerResource = nullptr;
    RDataDevice *dataDeviceResource = nullptr;
    std::list<GSeat*>::iterator clientLink;
};

#endif // SEATGLOBALPRIVATE_H
