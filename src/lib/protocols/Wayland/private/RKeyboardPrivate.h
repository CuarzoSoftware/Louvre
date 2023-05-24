#ifndef KEYBOARDRESOURCEPRIVATE_H
#define KEYBOARDRESOURCEPRIVATE_H

#include <protocols/Wayland/RKeyboard.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RKeyboard)

    static void resource_destroy(wl_resource *resource);
#if LOUVRE_SEAT_VERSION >= WL_KEYBOARD_RELEASE_SINCE_VERSION
    static void release(wl_client *client, wl_resource *resource);
#endif

    GSeat *seatGlobal;
    LastEventSerials serials;
};

#endif // KEYBOARDRESOURCEPRIVATE_H
