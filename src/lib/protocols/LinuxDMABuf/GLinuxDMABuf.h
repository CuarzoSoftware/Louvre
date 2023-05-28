#ifndef GLINUXDMABUF_H
#define GLINUXDMABUF_H

#include <LResource.h>

class Louvre::Protocols::LinuxDMABuf::GLinuxDMABuf : public LResource
{
public:
    GLinuxDMABuf(LCompositor *compositor,
                 wl_client *client,
                 const wl_interface *interface,
                 Int32 version,
                 UInt32 id,
                 const void *implementation,
                 wl_resource_destroy_func_t destroy);

    ~GLinuxDMABuf();

    bool format(UInt32 format) const;

    // Since 3
    bool modifier(UInt32 format, UInt32 mod_hi, UInt32 mod_lo) const;

    LPRIVATE_IMP(GLinuxDMABuf)
};

#endif // GLINUXDMABUF_H
