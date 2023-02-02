#ifndef SEATGLOBALPRIVATE_H
#define SEATGLOBALPRIVATE_H

#include <protocols/Wayland/SeatGlobal.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(SeatGlobal)

    // Implementation
    static void bind(wl_client *client, void *compositor, UInt32 version, UInt32 id);
    static void resource_destroy(wl_resource *resource);
    static void get_pointer(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_keyboard(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_touch(wl_client *client, wl_resource *resource, UInt32 id);
#if LOUVRE_SEAT_VERSION >= WL_SEAT_RELEASE_SINCE_VERSION
    static void release(wl_client *client, wl_resource *resource);
#endif

    KeyboardResource *keyboardResource = nullptr;
    PointerResource *pointerResource = nullptr;
    DataDeviceResource *dataDeviceResource = nullptr;
    std::list<SeatGlobal*>::iterator clientLink;
};

#endif // SEATGLOBALPRIVATE_H
