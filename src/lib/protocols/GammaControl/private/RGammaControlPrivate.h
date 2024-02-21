#ifndef RGAMMACONTROLPRIVATE_H
#define RGAMMACONTROLPRIVATE_H

#include <protocols/GammaControl/RGammaControl.h>

using namespace Louvre::Protocols::GammaControl;

LPRIVATE_CLASS(RGammaControl)
static void destroy(wl_client *client, wl_resource *resource);
static void set_gamma(wl_client *client, wl_resource *resource, Int32 fd);
Wayland::GOutput *gOutput = nullptr;
};

#endif // RGAMMACONTROLPRIVATE_H
