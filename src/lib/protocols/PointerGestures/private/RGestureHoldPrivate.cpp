#include <protocols/PointerGestures/private/RGestureHoldPrivate.h>

using namespace Louvre;

void RGestureHold::RGestureHoldPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
