#ifndef LDMABUFFER_H
#define LDMABUFFER_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Louvre/LTexture.h>

using namespace Louvre::Protocols::LinuxDMABuf;

class Louvre::LDMABuffer final : public LResource
{
public:

    static bool isDMABuffer(wl_resource *buffer) noexcept;

    const LDMAPlanes *planes() const noexcept
    {
        return m_dmaPlanes.get();
    }

    LTexture *texture() const noexcept
    {
        return m_texture;
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

private:
    friend class Louvre::Protocols::LinuxDMABuf::RLinuxBufferParams;
    friend class Louvre::LSurface;
    LDMABuffer(RLinuxBufferParams *bufferParamsRes, UInt32 id) noexcept;
    ~LDMABuffer() noexcept;
    std::unique_ptr<LDMAPlanes> m_dmaPlanes;
    LTexture *m_texture { nullptr };
};

#endif // LDMABUFFER_H
