#include <protocols/TearingControl/private/RTearingControlPrivate.h>
#include <protocols/TearingControl/tearing-control-v1.h>
#include <protocols/Wayland/RSurface.h>
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
    imp()->rSurface.reset(rSurface);
    rSurface->m_tearingControlRes.reset(this);
}

RTearingControl::~RTearingControl() {}

Wayland::RSurface *RTearingControl::surfaceResource() const
{
    return imp()->rSurface.get();
}

bool RTearingControl::preferVSync() const
{
    return imp()->preferVSync;
}
