#include <CZ/Louvre/Protocols/ImageCaptureSource/ext-image-capture-source-v1.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/RImageCaptureSource.h>

using namespace CZ::Protocols::ImageCaptureSource;

static const struct ext_image_capture_source_v1_interface imp
{
    .destroy = &RImageCaptureSource::destroy,
};

RImageCaptureSource::RImageCaptureSource
    (LClient *client,
     Int32 version,
     UInt32 id,
     LImageCaptureSourceType type,
     LOutput *output,
     LToplevelRole *toplevel
     ) noexcept
    :LResource
    (
        client,
        &ext_image_capture_source_v1_interface,
        version,
        id,
        &imp
    ),
    m_output(output),
    m_toplevel(toplevel),
    m_type(type)
{}

/******************** REQUESTS ********************/

void RImageCaptureSource::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
