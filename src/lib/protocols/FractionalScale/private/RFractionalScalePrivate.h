#ifndef RFRACTIONALSCALEPRIVATE_H
#define RFRACTIONALSCALEPRIVATE_H

#include <protocols/FractionalScale/RFractionalScale.h>

using namespace Louvre::Protocols::FractionalScale;

LPRIVATE_CLASS(RFractionalScale)
static void resource_destroy(wl_resource *resource);
static void destroy(wl_client *client, wl_resource *resource);
Wayland::RSurface *rSurface = nullptr;
Float32 lastScale = -1.f;
};

#endif // RFRACTIONALSCALEPRIVATE_H
