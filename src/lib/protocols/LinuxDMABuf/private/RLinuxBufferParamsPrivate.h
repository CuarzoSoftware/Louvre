#ifndef RLINUXBUFFERPARAMSPRIVATE_H
#define RLINUXBUFFERPARAMSPRIVATE_H

#include <protocols/LinuxDMABuf/RLinuxBufferParams.h>

using namespace Louvre::Protocols::LinuxDMABuf;

LPRIVATE_CLASS(RLinuxBufferParams)
    static void destroy(wl_client *client, wl_resource *resource);
    static void add(wl_client *client, wl_resource *resource, Int32 fd, UInt32 plane_idx, UInt32 offset, UInt32 stride, UInt32 modifier_hi, UInt32 modifier_lo);
    static void create(wl_client *client, wl_resource *resource, Int32 width, Int32 height, UInt32 format, UInt32 flags);
#if LOUVRE_LINUX_DMA_BUF_VERSION >= 2
    static void create_immed(wl_client *client, wl_resource *resource, UInt32 buffer_id, Int32 width, Int32 height, UInt32 format, UInt32 flags);
#endif

    LDMAPlanes *planes = nullptr;
};

#endif // RLINUXBUFFERPARAMSPRIVATE_H
