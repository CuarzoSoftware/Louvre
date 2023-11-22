#ifndef RLINUXBUFFERPARAMS_H
#define RLINUXBUFFERPARAMS_H

#include <LResource.h>

class Louvre::Protocols::LinuxDMABuf::RLinuxBufferParams : public LResource
{
public:
    RLinuxBufferParams(GLinuxDMABuf *gLinuxDMABuf, UInt32 id);
    ~RLinuxBufferParams();

    const LDMAPlanes *planes() const;

    // Since 1
    bool created(wl_resource *buffer);
    bool failed();

    LPRIVATE_IMP_UNIQUE(RLinuxBufferParams)
};

#endif // RLINUXBUFFERPARAMS_H
