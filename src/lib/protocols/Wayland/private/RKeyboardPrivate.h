#ifndef RKEYBOARDPRIVATE_H
#define RKEYBOARDPRIVATE_H

#include <protocols/Wayland/RKeyboard.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RKeyboard)
#if LOUVRE_WL_SEAT_VERSION >= 3
    static void release(wl_client *client, wl_resource *resource);
#endif

    GSeat *gSeat;
};

#endif // RKEYBOARDPRIVATE_H
