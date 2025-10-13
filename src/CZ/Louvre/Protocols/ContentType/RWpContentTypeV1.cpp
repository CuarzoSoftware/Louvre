#include <CZ/Louvre/Protocols/ContentType/content-type-v1.h>
#include <CZ/Louvre/Protocols/ContentType/RWpContentTypeV1.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>

using namespace CZ::Protocols::ContentType;

static const struct wp_content_type_v1_interface imp
{
    .destroy = &RWpContentTypeV1::destroy,
    .set_content_type = &RWpContentTypeV1::set_content_type
};

RWpContentTypeV1::RWpContentTypeV1
    (
        Wayland::RWlSurface *surfaceRes,
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

RWpContentTypeV1::~RWpContentTypeV1() noexcept
{
    if (surfaceRes())
        surfaceRes()->surface()->imp()->pending.contentType = RContentType::Graphics;
}

/******************** REQUESTS ********************/


void RWpContentTypeV1::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RWpContentTypeV1::set_content_type(wl_client */*client*/, wl_resource *resource, UInt32 type)
{
    auto &res { *static_cast<RWpContentTypeV1*>(wl_resource_get_user_data(resource)) };

    if (type > 3)
    {
        res.postError(0, "Ivalid content type value {}", type);
        return;
    }

    if (res.surfaceRes())
        res.surfaceRes()->surface()->imp()->pending.contentType = static_cast<RContentType>(type + 1);
}

