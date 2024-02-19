#ifndef RGESTUREHOLDPRIVATE_H
#define RGESTUREHOLDPRIVATE_H

#include <protocols/PointerGestures/RGestureHold.h>

using namespace Louvre::Protocols::Wayland;
using namespace Louvre::Protocols::PointerGestures;

LPRIVATE_CLASS(RGestureHold)
static void resource_destroy(wl_resource *resource);
static void destroy(wl_client *client, wl_resource *resource);

RPointer *rPointer = nullptr;
SerialEvents serialEvents;
};

#endif // RGESTUREHOLDPRIVATE_H
