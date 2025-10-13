#ifndef CZ_RZWPLINUXBUFFERPARAMSV1_H
#define CZ_RZWPLINUXBUFFERPARAMSV1_H

#include <CZ/Ream/RDMABufferInfo.h>
#include <CZ/Louvre/LResource.h>
#include <unordered_set>

class CZ::Protocols::LinuxDMABuf::RZwpLinuxBufferParamsV1 final : public LResource
{
public:

    const RDMABufferInfo &dmaInfo() const noexcept { return m_dmaInfo; }

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
    friend class CZ::Protocols::LinuxDMABuf::GZwpLinuxDmaBufV1;
    friend class CZ::LDMABuffer;
    RZwpLinuxBufferParamsV1(GZwpLinuxDmaBufV1 *linuxDMABufRes, UInt32 id) noexcept;
    ~RZwpLinuxBufferParamsV1() noexcept;
    bool createCommon(Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept;
    RDMABufferInfo m_dmaInfo {};
    std::shared_ptr<RImage> m_image;
    std::unordered_set<int> m_fds;
    bool m_isInvalid { false };
    bool m_used { false };
};

#endif // CZ_RZWPLINUXBUFFERPARAMSV1_H
