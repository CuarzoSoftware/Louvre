#ifndef RTOUCHPRIVATE_H
#define RTOUCHPRIVATE_H

#include <protocols/Wayland/RTouch.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(RTouch)
#if LOUVRE_WL_SEAT_VERSION >= 3
static void release(wl_client *client, wl_resource *resource);
#endif

GSeat *gSeat;
};

#endif // RTOUCHPRIVATE_H
