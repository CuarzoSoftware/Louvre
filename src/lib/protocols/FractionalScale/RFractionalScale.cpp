#include <protocols/FractionalScale/private/RFractionalScalePrivate.h>
#include <protocols/FractionalScale/fractional-scale-v1.h>
#include <protocols/Wayland/private/RSurfacePrivate.h>

using namespace Louvre;

static struct wp_fractional_scale_v1_interface wp_fractional_scale_v1_implementation
{
    .destroy = &RFractionalScale::RFractionalScalePrivate::destroy
};

RFractionalScale::RFractionalScale
    (
        Wayland::RSurface *rSurface,
        UInt32 id,
        Int32 version
    )
    :LResource
    (
        rSurface->client(),
        &wp_fractional_scale_v1_interface,
        version,
        id,
        &wp_fractional_scale_v1_implementation
    ),
    LPRIVATE_INIT_UNIQUE(RFractionalScale)
{
    imp()->rSurface.reset(rSurface);
    rSurface->imp()->rFractionalScale.reset(this);
}

RFractionalScale::~RFractionalScale() {}

RSurface *RFractionalScale::surfaceResource() const
{
    return imp()->rSurface.get();
}

bool RFractionalScale::preferredScale(Float32 scale)
{
    if (scale == imp()->lastScale)
        return true;

    imp()->lastScale = scale;
    wp_fractional_scale_v1_send_preferred_scale(resource(), (UInt32)(roundf(scale * 120.f)));
    return true;
}
