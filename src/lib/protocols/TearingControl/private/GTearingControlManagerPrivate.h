#ifndef GTEARINGCONTROLMANAGERPRIVATE_H
#define GTEARINGCONTROLMANAGERPRIVATE_H

#include <protocols/TearingControl/GTearingControlManager.h>
#include <protocols/TearingControl/tearing-control-v1.h>

using namespace Louvre::Protocols::TearingControl;

LPRIVATE_CLASS(GTearingControlManager)
static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
static void destroy(wl_client *client, wl_resource *resource);
static void get_tearing_control(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);
};

#endif // GTEARINGCONTROLMANAGERPRIVATE_H
