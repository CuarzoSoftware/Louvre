#include <protocols/XWaylandShell/xwayland-shell-v1.h>
#include <protocols/XWaylandShell/RXWaylandSurface.h>
#include <private/LSurfacePrivate.h>

using namespace Louvre;
using namespace Louvre::Protocols::XWaylandShell;

static const struct xwayland_surface_v1_interface imp
{
    .set_serial = &RXWaylandSurface::set_serial,
    .destroy = &RXWaylandSurface::destroy
};

RXWaylandSurface::RXWaylandSurface(Int32 version,
                                   LSurface *surface,
                                   UInt32 id
                                   ) noexcept
    :LResource
    (
        surface->client(),
        &xwayland_surface_v1_interface,
        version,
        id,
        &imp
        ),m_surface(surface)
{}

/******************** REQUESTS ********************/

void RXWaylandSurface::set_serial(wl_client */*client*/, wl_resource *resource, UInt32 serial_lo, UInt32 serial_hi)
{
    auto &res { *static_cast<RXWaylandSurface*>(wl_resource_get_user_data(resource)) };

    UInt64 serial = ;
}

void RXdgActivationToken::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RXdgActivationToken::done(const std::string &token) noexcept
{
    xdg_activation_token_v1_send_done(resource(), token.c_str());
}
