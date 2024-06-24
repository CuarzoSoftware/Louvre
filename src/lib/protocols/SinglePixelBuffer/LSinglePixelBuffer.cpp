#include <protocols/SinglePixelBuffer/LSinglePixelBuffer.h>

using namespace Louvre;

static const struct wl_buffer_interface imp
{
    .destroy = &LSinglePixelBuffer::destroy
};

LSinglePixelBuffer::LSinglePixelBuffer(
    LClient *client,
    Int32 version,
    UInt32 id,
    const UPixel32 &pixel)
    noexcept : LResource
    (
        client,
        &wl_buffer_interface,
        version,
        id,
        &imp
    ),
    m_pixel(pixel)
{}

bool LSinglePixelBuffer::isSinglePixelBuffer(wl_resource *buffer) noexcept
{
    return wl_resource_instance_of(buffer, &wl_buffer_interface, &imp);
}

/******************** REQUESTS ********************/

void LSinglePixelBuffer::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}
