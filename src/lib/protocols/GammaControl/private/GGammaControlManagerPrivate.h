#ifndef GGAMMACONTROLMANAGERPRIVATE_H
#define GGAMMACONTROLMANAGERPRIVATE_H

#include <protocols/GammaControl/GGammaControlManager.h>
#include <protocols/GammaControl/wlr-gamma-control-unstable-v1.h>

using namespace Louvre::Protocols::GammaControl;

LPRIVATE_CLASS(GGammaControlManager)
static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
static void resource_destroy(wl_resource *resource);
static void destroy(wl_client *client, wl_resource *resource);
static void get_gamma_control(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *output);
};

#endif // GGAMMACONTROLMANAGERPRIVATE_H
