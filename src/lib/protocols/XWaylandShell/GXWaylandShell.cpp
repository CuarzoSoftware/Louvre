#include <protocols/XWaylandShell/xwayland-shell-v1.h>
#include <protocols/XWaylandShell/GXWaylandShell.h>
#include <protocols/XWaylandShell/RXWaylandRole.h>
#include <protocols/Wayland/RSurface.h>
#include <LActivationTokenManager.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre;
using namespace Louvre::Protocols::XWaylandShell;

static const struct xwayland_shell_v1_interface imp
{
    .destroy = &GXWaylandShell::destroy,
    .get_xwayland_surface = &GXWaylandShell::get_xwayland_surface
};

void GXWaylandShell::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GXWaylandShell(client, version, id);
}

Int32 GXWaylandShell::maxVersion() noexcept
{
    return LOUVRE_XDG_ACTIVATION_VERSION;
}

const wl_interface *GXWaylandShell::interface() noexcept
{
    return &xwayland_shell_v1_interface;
}

GXWaylandShell::GXWaylandShell(
    wl_client *client,
    Int32 version,
    UInt32 id) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->xWaylandShellGlobals.emplace_back(this);
}

GXWaylandShell::~GXWaylandShell() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->xWaylandShellGlobals, this);
}

void GXWaylandShell::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXWaylandShell::get_xwayland_surface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    auto *res { static_cast<GXdgActivation*>(wl_resource_get_user_data(resource)) };
    new RXdgActivationToken(res, id);
}
