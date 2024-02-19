#ifndef RGESTUREPINCHPRIVATE_H
#define RGESTUREPINCHPRIVATE_H

#include <protocols/PointerGestures/RGesturePinch.h>

using namespace Louvre::Protocols::Wayland;
using namespace Louvre::Protocols::PointerGestures;

LPRIVATE_CLASS(RGesturePinch)
static void resource_destroy(wl_resource *resource);
static void destroy(wl_client *client, wl_resource *resource);

RPointer *rPointer = nullptr;
SerialEvents serialEvents;
};

#endif // RGESTUREPINCHPRIVATE_H
