#include <protocols/RelativePointer/private/RRelativePointerPrivate.h>

using namespace Louvre;

void RRelativePointer::RRelativePointerPrivate::resource_destroy(wl_resource *resource)
{
    RRelativePointer *rRelativePointer = (RRelativePointer*)wl_resource_get_user_data(resource);
    delete rRelativePointer;
}

void RRelativePointer::RRelativePointerPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
