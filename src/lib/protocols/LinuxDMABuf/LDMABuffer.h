#ifndef LDMABUFFER_H
#define LDMABUFFER_H

#include <LResource.h>
#include <LTexture.h>

using namespace Louvre::Protocols::LinuxDMABuf;

bool isDMABuffer(wl_resource *buffer);

class Louvre::LDMABuffer : public LResource
{
public:
    LDMABuffer(RLinuxBufferParams *rLinuxBufferParams, UInt32 id);
    ~LDMABuffer();

    const LDMAPlanes *planes() const;
    LTexture *texture() const;

    LPRIVATE_IMP_UNIQUE(LDMABuffer);
};

#endif // LDMABUFFER_H
