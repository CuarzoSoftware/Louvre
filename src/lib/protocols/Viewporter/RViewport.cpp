#include <protocols/Viewporter/GViewporter.h>
#include <protocols/Viewporter/RViewport.h>
#include <protocols/Wayland/RSurface.h>

using namespace Louvre::Protocols::Viewporter;

static const struct wp_viewport_interface imp
{
    .destroy = &RViewport::destroy,
    .set_source = &RViewport::set_source,
    .set_destination = &RViewport::set_destination
};

RViewport::RViewport
    (
        Wayland::RSurface *surfaceRes,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        surfaceRes->client(),
        &wp_viewport_interface,
        version,
        id,
        &imp
    ),
    m_surfaceRes(surfaceRes)
{
    surfaceRes->m_viewportRes.reset(this);
}

/******************** REQUESTS ********************/

void RViewport::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RViewport::set_source(wl_client */*client*/, wl_resource *resource, Float24 x, Float24 y, Float24 width, Float24 height) noexcept
{
    auto &res { *static_cast<RViewport*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, WP_VIEWPORT_ERROR_NO_SURFACE, "The wl_surface was destroyed.");
        return;
    }

    res.m_srcRect.setX(wl_fixed_to_double(x));
    res.m_srcRect.setY(wl_fixed_to_double(y));
    res.m_srcRect.setW(wl_fixed_to_double(width));
    res.m_srcRect.setH(wl_fixed_to_double(height));
}

void RViewport::set_destination(wl_client */*client*/, wl_resource *resource, Int32 width, Int32 height) noexcept
{
    auto &res { *static_cast<RViewport*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, WP_VIEWPORT_ERROR_NO_SURFACE, "The wl_surface was destroyed.");
        return;
    }

    res.m_dstSize.setW(width);
    res.m_dstSize.setH(height);
}
