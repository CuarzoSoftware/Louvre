#include <protocols/PointerGestures/private/RGestureHoldPrivate.h>

using namespace Louvre;

void RGestureHold::RGestureHoldPrivate::resource_destroy(wl_resource *resource)
{
    RGestureHold *rGestureHold = (RGestureHold*)wl_resource_get_user_data(resource);
    delete rGestureHold;
}

void RGestureHold::RGestureHoldPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
