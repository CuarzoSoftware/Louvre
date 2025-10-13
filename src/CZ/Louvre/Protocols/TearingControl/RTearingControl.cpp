#include <CZ/Louvre/Protocols/TearingControl/tearing-control-v1.h>
#include <CZ/Louvre/Protocols/TearingControl/GTearingControlManager.h>
#include <CZ/Louvre/Protocols/TearingControl/RTearingControl.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>

using namespace CZ::Protocols::TearingControl;

static const struct wp_tearing_control_v1_interface imp
{
    .set_presentation_hint = &RTearingControl::set_presentation_hint,
    .destroy = &RTearingControl::destroy,
};

RTearingControl::RTearingControl
    (
        Wayland::RWlSurface *surfaceRes,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        surfaceRes->client(),
        &wp_tearing_control_v1_interface,
        version,
        id,
        &imp
    ),
    m_surfaceRes(surfaceRes)
{
    surfaceRes->m_tearingControlRes.reset(this);
}

/******************** REQUESTS ********************/

void RTearingControl::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RTearingControl::set_presentation_hint(wl_client */*client*/, wl_resource *resource, UInt32 hint) noexcept
{
    auto &res { *static_cast<RTearingControl*>(wl_resource_get_user_data(resource)) };
    res.m_preferVSync = hint == WP_TEARING_CONTROL_V1_PRESENTATION_HINT_VSYNC;
}
