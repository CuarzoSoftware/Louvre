#ifndef RTEARINGCONTROLPRIVATE_H
#define RTEARINGCONTROLPRIVATE_H

#include <protocols/TearingControl/RTearingControl.h>

using namespace Louvre::Protocols::TearingControl;

LPRIVATE_CLASS(RTearingControl)
static void destroy(wl_client *client, wl_resource *resource);
static void set_presentation_hint(wl_client *client, wl_resource *resource, UInt32 hint);
Wayland::RSurface *rSurface { nullptr };
bool preferVSync { true };
};

#endif // RTEARINGCONTROLPRIVATE_H
