#include <protocols/TearingControl/private/RTearingControlPrivate.h>
#include <protocols/TearingControl/tearing-control-v1.h>
#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LCompositor.h>

using namespace Louvre;

static struct wp_tearing_control_v1_interface tearing_control_implementation =
{
    .set_presentation_hint = &RTearingControl::RTearingControlPrivate::set_presentation_hint,
    .destroy = &RTearingControl::RTearingControlPrivate::destroy,
};

RTearingControl::RTearingControl
    (
        Wayland::RSurface *rSurface,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        rSurface->client(),
        &wp_tearing_control_v1_interface,
        version,
        id,
        &tearing_control_implementation
    ),
    LPRIVATE_INIT_UNIQUE(RTearingControl)
{
    imp()->rSurface = rSurface;
    rSurface->imp()->rTearingControl = this;
}

RTearingControl::~RTearingControl()
{
    if (surfaceResource())
        surfaceResource()->imp()->rTearingControl = nullptr;
}

RSurface *RTearingControl::surfaceResource() const
{
    return imp()->rSurface;
}

bool RTearingControl::preferVSync() const
{
    return imp()->preferVSync;
}
