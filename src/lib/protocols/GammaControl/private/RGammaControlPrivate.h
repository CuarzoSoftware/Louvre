#ifndef RGAMMACONTROLPRIVATE_H
#define RGAMMACONTROLPRIVATE_H

#include <protocols/Wayland/GOutput.h>
#include <protocols/GammaControl/RGammaControl.h>

using namespace Louvre::Protocols::GammaControl;

LPRIVATE_CLASS(RGammaControl)
static void destroy(wl_client *client, wl_resource *resource);
static void set_gamma(wl_client *client, wl_resource *resource, Int32 fd);
LWeak<Wayland::GOutput> gOutput;
};

#endif // RGAMMACONTROLPRIVATE_H
