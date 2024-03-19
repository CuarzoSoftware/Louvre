#ifndef RFRACTIONALSCALEPRIVATE_H
#define RFRACTIONALSCALEPRIVATE_H

#include <protocols/Wayland/RSurface.h>
#include <protocols/FractionalScale/RFractionalScale.h>
#include <LWeak.h>

using namespace Louvre::Protocols::FractionalScale;

LPRIVATE_CLASS(RFractionalScale)
static void resource_destroy(wl_resource *resource);
static void destroy(wl_client *client, wl_resource *resource);
LWeak<Wayland::RSurface> rSurface;
Float32 lastScale = -1.f;
};

#endif // RFRACTIONALSCALEPRIVATE_H
