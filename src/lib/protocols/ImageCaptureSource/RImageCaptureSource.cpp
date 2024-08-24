#include <protocols/ImageCaptureSource/ext-image-capture-source-v1.h>
#include <protocols/ImageCaptureSource/RImageCaptureSource.h>

using namespace Louvre::Protocols::ImageCaptureSource;

static const struct ext_image_capture_source_v1_interface imp
{
    .destroy = &RImageCaptureSource::destroy,
};

RImageCaptureSource::RImageCaptureSource
    (LClient *client,
     Int32 version,
     UInt32 id,
     LImageCaptureSourceType type,
     LResource *source
     ) noexcept
    :LResource
    (
        client,
        &ext_image_capture_source_v1_interface,
        version,
        id,
        &imp
    ),
    m_source(source),
    m_type(type)
{}

/******************** REQUESTS ********************/

void RImageCaptureSource::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
