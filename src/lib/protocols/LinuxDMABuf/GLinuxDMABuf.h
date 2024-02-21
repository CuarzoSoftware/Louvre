#ifndef GLINUXDMABUF_H
#define GLINUXDMABUF_H

#include <LResource.h>

class Louvre::Protocols::LinuxDMABuf::GLinuxDMABuf : public LResource
{
public:
    GLinuxDMABuf(wl_client *client,
                 const wl_interface *interface,
                 Int32 version,
                 UInt32 id,
                 const void *implementation);

    ~GLinuxDMABuf();

    // Since 1
    bool format(UInt32 format);

    // Since 3
    bool modifier(UInt32 format, UInt32 mod_hi, UInt32 mod_lo);

    LPRIVATE_IMP_UNIQUE(GLinuxDMABuf)
};

#endif // GLINUXDMABUF_H
