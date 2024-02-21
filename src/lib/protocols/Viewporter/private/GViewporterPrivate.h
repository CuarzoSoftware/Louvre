#ifndef GVIEWPORTERPRIVATE_H
#define GVIEWPORTERPRIVATE_H

#include <protocols/Viewporter/GViewporter.h>
#include <protocols/Viewporter/viewporter.h>

using namespace Louvre::Protocols::Viewporter;

LPRIVATE_CLASS(GViewporter)
static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
static void destroy(wl_client *client, wl_resource *resource);
static void get_viewport(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);
};

#endif // GVIEWPORTERPRIVATE_H
