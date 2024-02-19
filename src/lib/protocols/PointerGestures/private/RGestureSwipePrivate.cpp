#include <protocols/PointerGestures/private/RGestureSwipePrivate.h>

using namespace Louvre;

void RGestureSwipe::RGestureSwipePrivate::resource_destroy(wl_resource *resource)
{
    RGestureSwipe *rGestureSwipe = (RGestureSwipe*)wl_resource_get_user_data(resource);
    delete rGestureSwipe;
}

void RGestureSwipe::RGestureSwipePrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
