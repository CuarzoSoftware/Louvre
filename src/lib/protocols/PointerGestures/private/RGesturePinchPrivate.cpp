#include <protocols/PointerGestures/private/RGesturePinchPrivate.h>

using namespace Louvre;

void RGesturePinch::RGesturePinchPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
