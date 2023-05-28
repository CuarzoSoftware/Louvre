#ifndef GLINUXDMABUFPRIVATE_H
#define GLINUXDMABUFPRIVATE_H

#include <protocols/LinuxDMABuf/GLinuxDMABuf.h>

using namespace Louvre::Protocols::LinuxDMABuf;
using namespace std;

LPRIVATE_CLASS(GLinuxDMABuf)
    static void bind(wl_client *client, void *compositor, UInt32 version, UInt32 id);
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void create_params(wl_client *client, wl_resource *resource, UInt32 id);

#if LOUVRE_LINUX_DMA_BUF_VERSION >= 4
    static void get_default_feedback(wl_client *client, wl_resource *resource, UInt32 id);
    static void get_surface_feedback(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface);
#endif

    list<GLinuxDMABuf*>::iterator clientLink;
};

#endif // GLINUXDMABUFPRIVATE_H
