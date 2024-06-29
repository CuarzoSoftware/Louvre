#include <protocols/ContentType/content-type-v1.h>
#include <protocols/ContentType/RContentType.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LSurfacePrivate.h>

using namespace Louvre::Protocols::ContentType;

static const struct wp_content_type_v1_interface imp
{
    .destroy = &RContentType::destroy,
    .set_content_type = &RContentType::set_content_type
};

RContentType::RContentType
    (
        Wayland::RSurface *surfaceRes,
        UInt32 id,
        Int32 version
    ) noexcept
    :LResource
    (
        surfaceRes->client(),
        &wp_content_type_v1_interface,
        version,
        id,
        &imp
        ),
    m_surfaceRes(surfaceRes)
{
    surfaceRes->m_contentTypeRes.reset(this);
}

RContentType::~RContentType() noexcept
{
    if (surfaceRes())
        surfaceRes()->surface()->imp()->pending.contentType = LContentTypeNone;
}

/******************** REQUESTS ********************/


void RContentType::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RContentType::set_content_type(wl_client */*client*/, wl_resource *resource, UInt32 type)
{
    auto &res { *static_cast<RContentType*>(wl_resource_get_user_data(resource)) };

    if (res.surfaceRes())
        res.surfaceRes()->surface()->imp()->pending.contentType = static_cast<LContentType>(type);
}

