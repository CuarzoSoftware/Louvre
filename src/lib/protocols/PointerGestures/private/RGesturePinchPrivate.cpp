#include <protocols/PointerGestures/private/RGesturePinchPrivate.h>

using namespace Louvre;

void RGesturePinch::RGesturePinchPrivate::resource_destroy(wl_resource *resource)
{
    RGesturePinch *rGesturePinch = (RGesturePinch*)wl_resource_get_user_data(resource);
    delete rGesturePinch;
}

void RGesturePinch::RGesturePinchPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
