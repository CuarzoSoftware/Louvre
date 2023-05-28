#ifndef RLINUXBUFFERPARAMS_H
#define RLINUXBUFFERPARAMS_H

#include <LResource.h>

class Louvre::Protocols::LinuxDMABuf::RLinuxBufferParams : public LResource
{
public:
    RLinuxBufferParams(GLinuxDMABuf *gLinuxDMABuf, UInt32 id);
    ~RLinuxBufferParams();

    const LDMAPlanes *planes() const;

    bool created(wl_resource *buffer) const;
    bool failed() const;

    LPRIVATE_IMP(RLinuxBufferParams)
};

#endif // RLINUXBUFFERPARAMS_H
