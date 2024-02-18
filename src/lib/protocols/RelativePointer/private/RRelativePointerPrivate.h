#ifndef RRELATIVEPOINTERPRIVATE_H
#define RRELATIVEPOINTERPRIVATE_H

#include <protocols/RelativePointer/RRelativePointer.h>

using namespace Louvre::Protocols::Wayland;
using namespace Louvre::Protocols::RelativePointer;

LPRIVATE_CLASS(RRelativePointer)
static void resource_destroy(wl_resource *resource);
static void destroy(wl_client *client, wl_resource *resource);

RPointer *rPointer = nullptr;
};

#endif // RRELATIVEPOINTERPRIVATE_H
