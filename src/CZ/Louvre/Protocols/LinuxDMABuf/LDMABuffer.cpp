#include <CZ/Louvre/Protocols/LinuxDMABuf/RLinuxBufferParams.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSurface.h>

using namespace Louvre::Protocols::Wayland;
using namespace Louvre;

static const struct wl_buffer_interface imp
{
    .destroy = &LDMABuffer::destroy
};

LDMABuffer::LDMABuffer
(
    RLinuxBufferParams *bufferParamsRes,
    UInt32 id
) noexcept
    :LResource
    (
        bufferParamsRes->client(),
        &wl_buffer_interface,
        1,
        id,
        &imp
    ),
    m_dmaPlanes(std::move(bufferParamsRes->m_dmaPlanes))
{}

LDMABuffer::~LDMABuffer() noexcept
{
    if (texture())
    {
        for (LSurface *s : compositor()->surfaces())
            if (s->texture() == texture())
            {
                texture()->m_pendingDelete = true;
                return;
            }

        delete m_texture;
    }
    else
    {
        for (UInt32 i = 0; i < planes()->num_fds; i++)
            close(planes()->fds[i]);
    }
}

bool LDMABuffer::isDMABuffer(wl_resource *buffer) noexcept
{
    return wl_resource_instance_of(buffer, &wl_buffer_interface, &imp);
}

/******************** REQUESTS ********************/

void LDMABuffer::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
