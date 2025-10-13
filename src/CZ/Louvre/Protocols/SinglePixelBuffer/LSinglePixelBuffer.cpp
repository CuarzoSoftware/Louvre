#include <CZ/Louvre/Protocols/SinglePixelBuffer/LSinglePixelBuffer.h>

using namespace CZ;

static const struct wl_buffer_interface imp
{
    .destroy = &LSinglePixelBuffer::destroy
};

LSinglePixelBuffer::LSinglePixelBuffer(LClient *client,
                                       Int32 version,
                                       UInt32 id,
                                       std::shared_ptr<RImage> image)
    noexcept : LResource
    (
        client,
        &wl_buffer_interface,
        version,
        id,
        &imp
    ),
    image(image)
{
    assert(image);
}

bool LSinglePixelBuffer::isSinglePixelBuffer(wl_resource *buffer) noexcept
{
    return wl_resource_instance_of(buffer, &wl_buffer_interface, &imp);
}

/******************** REQUESTS ********************/

void LSinglePixelBuffer::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}
