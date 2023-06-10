#ifndef RKEYBOARDPRIVATE_H
#define RKEYBOARDPRIVATE_H

#include <protocols/Wayland/RKeyboard.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RKeyboard)
    static void resource_destroy(wl_resource *resource);
#if LOUVRE_WL_SEAT_VERSION >= WL_KEYBOARD_RELEASE_SINCE_VERSION
    static void release(wl_client *client, wl_resource *resource);
#endif

    GSeat *gSeat;
    LastEventSerials serials;
};

#endif // RKEYBOARDPRIVATE_H
