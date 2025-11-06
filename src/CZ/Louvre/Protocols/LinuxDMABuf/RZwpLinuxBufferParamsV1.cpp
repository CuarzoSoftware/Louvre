#include <CZ/Louvre/Protocols/LinuxDMABuf/linux-dmabuf-v1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/RZwpLinuxBufferParamsV1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/GZwpLinuxDmaBufV1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Core/CZBitset.h>

using namespace CZ::Protocols::LinuxDMABuf;

static const struct zwp_linux_buffer_params_v1_interface imp
{
    .destroy = &RZwpLinuxBufferParamsV1::destroy,
    .add = &RZwpLinuxBufferParamsV1::add,
    .create = &RZwpLinuxBufferParamsV1::create,
#if LOUVRE_LINUX_DMA_BUF_VERSION >= 2
    .create_immed = &RZwpLinuxBufferParamsV1::create_immed
#endif
};

RZwpLinuxBufferParamsV1::RZwpLinuxBufferParamsV1(
    GZwpLinuxDmaBufV1 *linuxDMABufRes,
    UInt32 id
) noexcept
    :LResource
    (
        linuxDMABufRes->client(),
        &zwp_linux_buffer_params_v1_interface,
        linuxDMABufRes->version(),
        id,
        &imp
    )
{
    m_fds.reserve(4);
}

RZwpLinuxBufferParamsV1::~RZwpLinuxBufferParamsV1() noexcept
{
    // This means that no RImage took ownership of the fds, so we must closed them here
    if (m_isInvalid)
    {
        while (!m_fds.empty())
        {
            close(*m_fds.begin());
            m_fds.erase(m_fds.begin());
        }
    }
}

int RZwpLinuxBufferParamsV1::createCommon(Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept
{
    if (m_used)
    {
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED, "The dmabuf_batch object has already been used to create a wl_buffer.");
        return -1;
    }

    m_used = true;

    if (flags)
    {
        m_isInvalid = true;
        failed();
        return 0;
    }

    if (width <= 0 || height <= 0)
    {
        m_isInvalid = true;
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS, "Invalid wl_buffer size {}x{}.", width, height);
        return -1;
    }

    if (m_dmaInfo.planeCount == 0)
    {
        m_isInvalid = true;
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE, "Invalid number of planes {}.", m_dmaInfo.planeCount);
        return -1;
    }

    for (int i = 0; i < 4; i++)
    {
        if (i < m_dmaInfo.planeCount && m_dmaInfo.fd[i] < 0)
        {
            m_isInvalid = true;
            postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE, "Missing plane {}/{}.", i + 1, m_dmaInfo.planeCount);
            return -1;
        }
    }

    m_dmaInfo.format = format;
    m_dmaInfo.width = width;
    m_dmaInfo.height = height;

    auto ream { RCore::Get() };
    RImageConstraints cons {};
    cons.allocator = ream->mainDevice();
    cons.caps[ream->mainDevice()] = RImageCap_Src; // At least the main device should be able to sample from it
    m_image = RImage::FromDMA(m_dmaInfo, CZOwn::Own, &cons);

    if (!m_image)
    {
        /* Nvidia drivers keep too many open files, if one of the fds is larger than 1020 the compositor may have reached
         * the default limit (1024) */
        for (int i = 0; i < m_dmaInfo.planeCount; i++)
        {
            if (m_dmaInfo.fd[i] >= 1020)
            {
                wl_client_post_no_memory(client()->client());
                return -1;
            }
        }

        failed();
        return 0;
    }

    return 1;
}

/******************** REQUESTS ********************/

void RZwpLinuxBufferParamsV1::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RZwpLinuxBufferParamsV1::add(wl_client */*client*/,
                            wl_resource *resource,
                            Int32 fd, UInt32 plane_idx,
                            UInt32 offset, UInt32 stride,
                            UInt32 modifier_hi, UInt32 modifier_lo) noexcept
{
    auto *res { static_cast<RZwpLinuxBufferParamsV1*>(wl_resource_get_user_data(resource)) };

    if (res->m_used)
    {
        close(fd);
        res->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED, "The dmabuf_batch object has already been used to create a wl_buffer.");
        return;
    }

    res->m_fds.emplace(fd);

    if (res->m_isInvalid)
        return;

    if (plane_idx >= 4)
    {
        res->m_isInvalid = true;
        res->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX, "Invalid DMA plane index. Max number of planes is {}.", 4);
        return;
    }

    if (res->dmaInfo().fd[plane_idx] >= 0)
    {
        res->m_isInvalid = true;
        res->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_SET, "Plane {} already set.", plane_idx);
        return;
    }

    res->m_dmaInfo.fd[plane_idx] = fd;
    res->m_dmaInfo.offset[plane_idx] = offset;
    res->m_dmaInfo.stride[plane_idx] = stride;
    const RModifier mod { (static_cast<UInt64>(modifier_hi) << 32) | static_cast<UInt64>(modifier_lo) };

    if (res->m_dmaInfo.planeCount > 0)
    {
        if (res->m_dmaInfo.modifier != mod)
        {
            res->m_isInvalid = true;

            if (res->version() >= 5)
            {
                res->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_FORMAT,
                               "All modifiers must be the same ({} != {})",
                               RDRMFormat::ModifierName(mod), RDRMFormat::ModifierName(res->m_dmaInfo.modifier));
                return;
            }
        }
    }
    else
        res->m_dmaInfo.modifier = mod;

    res->m_dmaInfo.planeCount++;
}

void RZwpLinuxBufferParamsV1::create(wl_client */*client*/, wl_resource *resource, Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept
{
    auto *res { static_cast<RZwpLinuxBufferParamsV1*>(wl_resource_get_user_data(resource)) };
    if (res->createCommon(width, height, format, flags) == 1)
        res->created((new LDMABuffer(std::move(res->m_image), res->client(), 0))->resource());

    // Ignore the leak warning, the buffer is destroyed when the client releases it or is disconnected
}

#if LOUVRE_LINUX_DMA_BUF_VERSION >= 2
void RZwpLinuxBufferParamsV1::create_immed(wl_client */*client*/, wl_resource *resource, UInt32 buffer_id,
                                     Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept
{
    auto *res { static_cast<RZwpLinuxBufferParamsV1*>(wl_resource_get_user_data(resource)) };
    if (res->createCommon(width, height, format, flags) == -1)
        return;
    new LDMABuffer(std::move(res->m_image), res->client(), buffer_id);
}
#endif

/******************** EVENTS ********************/

void RZwpLinuxBufferParamsV1::created(wl_resource *buffer) noexcept
{
    zwp_linux_buffer_params_v1_send_created(resource(), buffer);
}

void RZwpLinuxBufferParamsV1::failed() noexcept
{
    zwp_linux_buffer_params_v1_send_failed(resource());
}
