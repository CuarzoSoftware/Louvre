#include <protocols/FractionalScale/fractional-scale-v1.h>
#include <protocols/FractionalScale/RFractionalScale.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LSurfacePrivate.h>

using namespace Louvre::Protocols::FractionalScale;

static const struct wp_fractional_scale_v1_interface imp
{
    .destroy = &RFractionalScale::destroy
};

RFractionalScale::RFractionalScale
    (
        Wayland::RSurface *surfaceRes,
        UInt32 id,
        Int32 version
    ) noexcept
    :LResource
    (
        surfaceRes->client(),
        &wp_fractional_scale_v1_interface,
        version,
        id,
        &imp
    ),
    m_surfaceRes(surfaceRes)
{
    surfaceRes->m_fractionalScaleRes.reset(this);
    surfaceRes->surface()->imp()->sendPreferredScale();
}

/******************** REQUESTS ********************/


void RFractionalScale::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RFractionalScale::preferredScale(Float32 scale) noexcept
{
    if (scale == m_lastScale)
        return;

    m_lastScale = scale;
    wp_fractional_scale_v1_send_preferred_scale(resource(), (UInt32)(roundf(scale * 120.f)));
}
