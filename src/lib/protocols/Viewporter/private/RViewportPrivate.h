#ifndef RVIEWPORTPRIVATE_H
#define RVIEWPORTPRIVATE_H

#include <protocols/Viewporter/RViewport.h>
#include <LRect.h>

using namespace Louvre::Protocols::Viewporter;

LPRIVATE_CLASS(RViewport)
static void destroy(wl_client *client, wl_resource *resource);
static void set_source(wl_client *client, wl_resource *resource, Float24 x, Float24 y, Float24 width, Float24 height);
static void set_destination(wl_client *client, wl_resource *resource, Int32 width, Int32 height);
Wayland::RSurface *rSurface { nullptr };
LSize dstSize { -1, -1 };
LRectF srcRect { -1, -1, -1, -1 };
};

#endif // RVIEWPORTPRIVATE_H
