#include <protocols/LinuxDMABuf/linux-dmabuf-unstable-v1.h>
#include <protocols/LinuxDMABuf/private/RLinuxBufferParamsPrivate.h>
#include <protocols/LinuxDMABuf/private/LDMABufferPrivate.h>
#include <LTexture.h>

void RLinuxBufferParams::RLinuxBufferParamsPrivate::resource_destroy(wl_resource *resource)
{
    RLinuxBufferParams *rLinuxBufferParams = (RLinuxBufferParams*)wl_resource_get_user_data(resource);
    delete rLinuxBufferParams;
}

void RLinuxBufferParams::RLinuxBufferParamsPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RLinuxBufferParams::RLinuxBufferParamsPrivate::add(wl_client *client,
                                                        wl_resource *resource,
                                                        Int32 fd, UInt32 plane_idx,
                                                        UInt32 offset, UInt32 stride,
                                                        UInt32 modifier_hi, UInt32 modifier_lo)
{
    L_UNUSED(client);
    RLinuxBufferParams *rLinuxBufferParams = (RLinuxBufferParams*)wl_resource_get_user_data(resource);

    if (!rLinuxBufferParams->planes())
    {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED,
                               "The dmabuf_batch object has already been used to create a wl_buffer.");
        return;
    }

    if (plane_idx >= LOUVRE_MAX_DMA_PLANES)
    {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX,
                               "Invalid DMA plane index. Max number of planes is %d.",
                                LOUVRE_MAX_DMA_PLANES);
        return;
    }

    if (plane_idx >= LOUVRE_MAX_DMA_PLANES)
    {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX,
                               "Invalid DMA plane index. Max number of planes is %d.",
                               LOUVRE_MAX_DMA_PLANES);
        return;
    }

    UInt32 planeCount = plane_idx + 1;
    if (rLinuxBufferParams->planes()->num_fds < planeCount)
        rLinuxBufferParams->imp()->planes->num_fds = planeCount;

    rLinuxBufferParams->imp()->planes->fds[plane_idx] = fd;
    rLinuxBufferParams->imp()->planes->offsets[plane_idx] = offset;
    rLinuxBufferParams->imp()->planes->strides[plane_idx] = stride;
    rLinuxBufferParams->imp()->planes->modifiers[plane_idx] = ((UInt64)modifier_hi << 32) | modifier_lo;
}

void RLinuxBufferParams::RLinuxBufferParamsPrivate::create(wl_client *client,
                                                           wl_resource *resource,
                                                           Int32 width, Int32 height,
                                                           UInt32 format, UInt32 flags)
{
    RLinuxBufferParams *rLinuxBufferParams = (RLinuxBufferParams*)wl_resource_get_user_data(resource);

    L_UNUSED(client);

    if (!rLinuxBufferParams->planes())
    {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED,
                               "The dmabuf_batch object has already been used to create a wl_buffer.");
        return;
    }

    if (flags)
    {
        rLinuxBufferParams->failed();
        delete rLinuxBufferParams->imp()->planes;
        rLinuxBufferParams->imp()->planes = nullptr;
        return;
    }

    if (width <= 0 || height <= 0)
    {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS,
                               "Invalid wl_buffer size.");
        return;
    }

    rLinuxBufferParams->imp()->planes->format = format;
    rLinuxBufferParams->imp()->planes->width = width;
    rLinuxBufferParams->imp()->planes->height = height;

    LDMABuffer *buffer = new LDMABuffer(rLinuxBufferParams, 0);

    rLinuxBufferParams->imp()->planes = nullptr;

    rLinuxBufferParams->created(buffer->resource());
}

#if LOUVRE_LINUX_DMA_BUF_VERSION >= 2
void RLinuxBufferParams::RLinuxBufferParamsPrivate::create_immed(wl_client *client,
                                                                 wl_resource *resource,
                                                                 UInt32 buffer_id,
                                                                 Int32 width,
                                                                 Int32 height,
                                                                 UInt32 format,
                                                                 UInt32 flags)
{
    RLinuxBufferParams *rLinuxBufferParams = (RLinuxBufferParams*)wl_resource_get_user_data(resource);

    L_UNUSED(client);

    if (!rLinuxBufferParams->planes())
    {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED,
                               "The dmabuf_batch object has already been used to create a wl_buffer.");
        return;
    }

    if (flags)
    {
        rLinuxBufferParams->failed();
        delete rLinuxBufferParams->imp()->planes;
        rLinuxBufferParams->imp()->planes = nullptr;
        return;
    }

    if (width <= 0 || height <= 0)
    {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS,
                               "Invalid wl_buffer size.");
        return;
    }

    rLinuxBufferParams->imp()->planes->format = format;
    rLinuxBufferParams->imp()->planes->width = width;
    rLinuxBufferParams->imp()->planes->height = height;

    new LDMABuffer(rLinuxBufferParams, buffer_id);

    rLinuxBufferParams->imp()->planes = nullptr;
}
#endif
