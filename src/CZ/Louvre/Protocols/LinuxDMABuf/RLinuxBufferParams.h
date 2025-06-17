#ifndef RLINUXBUFFERPARAMS_H
#define RLINUXBUFFERPARAMS_H

#include <CZ/Louvre/LResource.h>
#include <memory>

class Louvre::Protocols::LinuxDMABuf::RLinuxBufferParams final : public LResource
{
public:

    const LDMAPlanes *dmaPlanes() const noexcept
    {
        return m_dmaPlanes.get();
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *, wl_resource *resource) noexcept;
    static void add(wl_client *client, wl_resource *resource, Int32 fd, UInt32 plane_idx, UInt32 offset, UInt32 stride, UInt32 modifier_hi, UInt32 modifier_lo) noexcept;
    static void create(wl_client *client, wl_resource *resource, Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept;
#if LOUVRE_LINUX_DMA_BUF_VERSION >= 2
    static void create_immed(wl_client *client, wl_resource *resource, UInt32 buffer_id, Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept;
#endif

    /******************** EVENTS ********************/

    // Since 1
    void created(wl_resource *buffer) noexcept;
    void failed() noexcept;

private:
    friend class Louvre::Protocols::LinuxDMABuf::GLinuxDMABuf;
    friend class Louvre::LDMABuffer;
    RLinuxBufferParams(GLinuxDMABuf *linuxDMABufRes, UInt32 id) noexcept;
    ~RLinuxBufferParams() noexcept = default;
    bool createCommon(Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept;
    std::unique_ptr<LDMAPlanes> m_dmaPlanes;
};

#endif // RLINUXBUFFERPARAMS_H
