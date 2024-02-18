#ifndef GRELATIVEPOINTERMANAGERPRIVATE_H
#define GRELATIVEPOINTERMANAGERPRIVATE_H

#include <protocols/RelativePointer/GRelativePointerManager.h>
#include <protocols/RelativePointer/relative-pointer-unstable-v1.h>

using namespace Louvre::Protocols::RelativePointer;

LPRIVATE_CLASS(GRelativePointerManager)
static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
static void resource_destroy(wl_resource *resource);
static void destroy(wl_client *client, wl_resource *resource);
static void get_relative_pointer(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer);
};

#endif // GRELATIVEPOINTERMANAGERPRIVATE_H
