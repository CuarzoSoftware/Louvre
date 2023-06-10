#include <protocols/Wayland/private/RCallbackPrivate.h>

void RCallback::RCallbackPrivate::resource_destroy(wl_resource *resource)
{
    RCallback *rCallback = (RCallback*)wl_resource_get_user_data(resource);
    delete rCallback;
}
