#include <CZ/Louvre/Protocols/LinuxDMABuf/RZwpLinuxBufferParamsV1.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Roles/LSurface.h>

using namespace CZ::Protocols::Wayland;
using namespace CZ;

static const struct wl_buffer_interface imp
{
    .destroy = &LDMABuffer::destroy
};

LDMABuffer::LDMABuffer
    (
        std::shared_ptr<RImage> &&image,
        LClient *client,
        UInt32 id
    ) noexcept
    :LResource
    (
        client,
        &wl_buffer_interface,
        1,
        id,
        &imp
    ),
    m_image(std::move(image))
{
    // RImage can only be nullptr if 'failed' was sent after 'create_immed'.
    // We expect the client to destroy the buffer and try again.
    // If the client commits the buffer or uses it elsewhere, we kill it.
}

LDMABuffer::~LDMABuffer() noexcept {}

bool LDMABuffer::isDMABuffer(wl_resource *buffer) noexcept
{
    return wl_resource_instance_of(buffer, &wl_buffer_interface, &imp);
}

/******************** REQUESTS ********************/

void LDMABuffer::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
