#include <protocols/LinuxDMABuf/private/LDMABufferPrivate.h>
#include <protocols/LinuxDMABuf/private/RLinuxBufferParamsPrivate.h>

#include <LCompositor.h>
#include <private/LSurfacePrivate.h>
#include <private/LTexturePrivate.h>

using namespace Louvre;

static struct wl_buffer_interface wl_dma_buffer_implementation
{
    .destroy = &LDMABuffer::LDMABufferPrivate::destroy
};

LDMABuffer::LDMABuffer
(
    RLinuxBufferParams *rLinuxBufferParams,
    UInt32 id
)
    :LResource
    (
        rLinuxBufferParams->client(),
        &wl_buffer_interface,
        1,
        id,
        &wl_dma_buffer_implementation
    ),
    LPRIVATE_INIT_UNIQUE(LDMABuffer)
{
    imp()->planes = rLinuxBufferParams->imp()->planes;
}

LDMABuffer::~LDMABuffer()
{
    if (texture())
    {
        for (LSurface *s : compositor()->surfaces())
            if (s->texture() == texture())
            {
                texture()->imp()->pendingDelete = true;
                goto skipDeleteTexture;
            }

        delete imp()->texture;
    }

    skipDeleteTexture:
    delete imp()->planes;
}

const LDMAPlanes *LDMABuffer::planes() const
{
    return imp()->planes;
}

LTexture *LDMABuffer::texture() const
{
    return imp()->texture;
}

bool isDMABuffer(wl_resource *buffer)
{
    return wl_resource_instance_of(buffer, &wl_buffer_interface, &wl_dma_buffer_implementation);
}
