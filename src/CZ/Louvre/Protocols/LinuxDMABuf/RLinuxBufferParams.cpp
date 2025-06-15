#include <CZ/Louvre/Protocols/LinuxDMABuf/linux-dmabuf-v1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/RLinuxBufferParams.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/GLinuxDMABuf.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>

using namespace Louvre::Protocols::LinuxDMABuf;

static const struct zwp_linux_buffer_params_v1_interface imp
{
    .destroy = &RLinuxBufferParams::destroy,
    .add = &RLinuxBufferParams::add,
    .create = &RLinuxBufferParams::create,
#if LOUVRE_LINUX_DMA_BUF_VERSION >= 2
    .create_immed = &RLinuxBufferParams::create_immed
#endif
};

RLinuxBufferParams::RLinuxBufferParams(
    GLinuxDMABuf *linuxDMABufRes,
    UInt32 id
) noexcept
    :LResource
    (
        linuxDMABufRes->client(),
        &zwp_linux_buffer_params_v1_interface,
        linuxDMABufRes->version(),
        id,
        &imp
    ),
    m_dmaPlanes(std::make_unique<LDMAPlanes>())
{}

bool RLinuxBufferParams::createCommon(Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept
{
    if (!dmaPlanes())
    {
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED,
                "The dmabuf_batch object has already been used to create a wl_buffer.");
        return false;
    }

    if (flags)
    {
        failed();
        m_dmaPlanes.reset();
        return false;
    }

    if (width <= 0 || height <= 0)
    {
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS, "Invalid wl_buffer size.");
        return false;
    }

    m_dmaPlanes->format = format;
    m_dmaPlanes->width = width;
    m_dmaPlanes->height = height;
    return true;
}

/******************** REQUESTS ********************/

void RLinuxBufferParams::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RLinuxBufferParams::add(wl_client */*client*/,
                            wl_resource *resource,
                            Int32 fd, UInt32 plane_idx,
                            UInt32 offset, UInt32 stride,
                            UInt32 modifier_hi, UInt32 modifier_lo) noexcept
{
    auto *res { static_cast<RLinuxBufferParams*>(wl_resource_get_user_data(resource)) };

    if (!res->dmaPlanes())
    {
        res->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED,
                               "The dmabuf_batch object has already been used to create a wl_buffer.");
        return;
    }

    if (plane_idx >= LOUVRE_MAX_DMA_PLANES)
    {
        res->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX,
                               "Invalid DMA plane index. Max number of planes is %d.",
                               LOUVRE_MAX_DMA_PLANES);
        return;
    }

    if (plane_idx >= LOUVRE_MAX_DMA_PLANES)
    {
        res->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX,
                               "Invalid DMA plane index. Max number of planes is %d.",
                               LOUVRE_MAX_DMA_PLANES);
        return;
    }

    const UInt32 planeCount { plane_idx + 1 };
    if (res->dmaPlanes()->num_fds < planeCount)
        res->m_dmaPlanes->num_fds = planeCount;

    res->m_dmaPlanes->fds[plane_idx] = fd;
    res->m_dmaPlanes->offsets[plane_idx] = offset;
    res->m_dmaPlanes->strides[plane_idx] = stride;
    res->m_dmaPlanes->modifiers[plane_idx] = (static_cast<UInt64>(modifier_hi) << 32) | static_cast<UInt64>(modifier_lo);
}

void RLinuxBufferParams::create(wl_client */*client*/, wl_resource *resource, Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept
{
    auto *res { static_cast<RLinuxBufferParams*>(wl_resource_get_user_data(resource)) };
    if (res->createCommon(width, height, format, flags))
        res->created((new LDMABuffer(res, 0))->resource());

    // Ignore the leak warning, the buffer is destroyed when the client releases it or is disconnected
}

#if LOUVRE_LINUX_DMA_BUF_VERSION >= 2
void RLinuxBufferParams::create_immed(wl_client */*client*/, wl_resource *resource, UInt32 buffer_id,
                                     Int32 width, Int32 height, UInt32 format, UInt32 flags) noexcept
{
    auto *res { static_cast<RLinuxBufferParams*>(wl_resource_get_user_data(resource)) };
    if (res->createCommon(width, height, format, flags))
        new LDMABuffer(res, buffer_id);
}
#endif

/******************** EVENTS ********************/

void RLinuxBufferParams::created(wl_resource *buffer) noexcept
{
    zwp_linux_buffer_params_v1_send_created(resource(), buffer);
}

void RLinuxBufferParams::failed() noexcept
{
    zwp_linux_buffer_params_v1_send_failed(resource());
}
