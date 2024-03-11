#ifndef GOUTPUTPRIVATE_H
#define GOUTPUTPRIVATE_H

#include <protocols/Wayland//GOutput.h>
#include <LOutput.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(GOutput)

static void bind(wl_client *client, void *output, UInt32 version, UInt32 id);
#if LOUVRE_WL_OUTPUT_VERSION >= 3
static void release(wl_client *client, wl_resource *resource);
#endif

LWeak<LOutput> output;
std::vector<GammaControl::RGammaControl*> gammaControlResources;
};

#endif // GOUTPUTPRIVATE_H
