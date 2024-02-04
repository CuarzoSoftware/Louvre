#ifndef GOUTPUTPRIVATE_H
#define GOUTPUTPRIVATE_H

#include <protocols/Wayland//GOutput.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(GOutput)

static void bind(wl_client *client, void *output, UInt32 version, UInt32 id);
static void resource_destroy(wl_resource *resource);
#if LOUVRE_WL_OUTPUT_VERSION >= 3
static void release(wl_client *client, wl_resource *resource);
#endif

LOutput *lOutput = nullptr;
std::vector<GammaControl::RGammaControl*> gammaControlResources;
};

#endif // GOUTPUTPRIVATE_H
