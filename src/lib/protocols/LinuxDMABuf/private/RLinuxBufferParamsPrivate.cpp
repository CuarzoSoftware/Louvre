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
                                                        int fd, UInt32 plane_idx,
                                                        UInt32 offset, UInt32 stride,
                                                        UInt32 modifier_hi, UInt32 modifier_lo)
{
    L_UNUSED(client);
    RLinuxBufferParams *rLinuxBufferParams = (RLinuxBufferParams*)wl_resource_get_user_data(resource);

    if (!rLinuxBufferParams->planes())
        return;

    if (plane_idx > 3)
        return;

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

    if (flags)
        return;

    if (!rLinuxBufferParams->planes())
        return;

    if (width <= 0 || height <= 0)
        return;

    rLinuxBufferParams->imp()->planes->format = format;
    rLinuxBufferParams->imp()->planes->width = width;
    rLinuxBufferParams->imp()->planes->height = height;

    LDMABuffer *buffer = new LDMABuffer(rLinuxBufferParams, 0);

    rLinuxBufferParams->imp()->planes = nullptr;

    rLinuxBufferParams->created(buffer->resource());
}

#if LOUVRE_LINUX_DMA_BUF_VERSION >= 2
void RLinuxBufferParams::RLinuxBufferParamsPrivate::create_immed(wl_client *client, wl_resource *resource, UInt32 buffer_id, Int32 width, Int32 height, UInt32 format, UInt32 flags)
{
    RLinuxBufferParams *rLinuxBufferParams = (RLinuxBufferParams*)wl_resource_get_user_data(resource);

    L_UNUSED(client);

    if (flags)
        return;

    if (!rLinuxBufferParams->planes())
        return;

    if (width <= 0 || height <= 0)
        return;

    rLinuxBufferParams->imp()->planes->format = format;
    rLinuxBufferParams->imp()->planes->width = width;
    rLinuxBufferParams->imp()->planes->height = height;

    new LDMABuffer(rLinuxBufferParams, buffer_id);

    rLinuxBufferParams->imp()->planes = nullptr;

    //rLinuxBufferParams->created(buffer->resource());
}
#endif
