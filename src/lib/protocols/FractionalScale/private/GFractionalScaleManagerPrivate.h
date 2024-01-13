#ifndef GFRACTIONALSCALEMANAGERPRIVATE_H
#define GFRACTIONALSCALEMANAGERPRIVATE_H

#include <protocols/FractionalScale/GFractionalScaleManager.h>
#include <protocols/FractionalScale/fractional-scale-v1.h>

using namespace Louvre::Protocols::FractionalScale;

LPRIVATE_CLASS(GFractionalScaleManager)
static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
static void resource_destroy(wl_resource *resource);
static void destroy(wl_client *client, wl_resource *resource);
static void get_fractional_scale(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);

std::list<GFractionalScaleManager*>::iterator clientLink;
};

#endif // GFRACTIONALSCALEMANAGERPRIVATE_H
