#include <protocols/RelativePointer/private/RRelativePointerPrivate.h>

using namespace Louvre;

void RRelativePointer::RRelativePointerPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
