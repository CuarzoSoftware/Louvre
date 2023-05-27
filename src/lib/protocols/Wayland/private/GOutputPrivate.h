#ifndef GOUTPUTPRIVATE_H
#define GOUTPUTPRIVATE_H

#include <protocols/Wayland//GOutput.h>

using namespace Louvre::Protocols::Wayland;

LPRIVATE_CLASS(GOutput)

static void bind(wl_client *client, void *output, UInt32 version, UInt32 id);
static void resource_destroy(wl_resource *resource);

#if LOUVRE_OUTPUT_VERSION >= WL_OUTPUT_RELEASE_SINCE_VERSION
static void release(wl_client *client, wl_resource *resource);
#endif

LOutput *output = nullptr;
std::list<GOutput*>::iterator clientLink;
};

#endif // GOUTPUTPRIVATE_H
