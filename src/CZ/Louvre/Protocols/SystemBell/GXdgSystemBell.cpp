#include <CZ/Louvre/Protocols/SystemBell/xdg-system-bell-v1.h>
#include <CZ/Louvre/Protocols/SystemBell/GXdgSystemBell.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ::Protocols::SystemBell;
using namespace CZ;

static const struct xdg_system_bell_v1_interface imp
{
    .destroy = &GXdgSystemBell::destroy,
    .ring = &GXdgSystemBell::ring
};

LGLOBAL_INTERFACE_IMP(GXdgSystemBell, LOUVRE_XDG_SYSTEM_BELL_VERSION, xdg_system_bell_v1_interface)

bool GXdgSystemBell::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.XdgSystemBell)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.XdgSystemBell;
    return true;
}

GXdgSystemBell::GXdgSystemBell(
    wl_client *client,
    Int32 version,
    UInt32 id
    )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{}

/******************** REQUESTS ********************/

void GXdgSystemBell::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GXdgSystemBell::ring(wl_client */*client*/, wl_resource *resource, wl_resource *surface) noexcept
{
    auto &res { *static_cast<GXdgSystemBell*>(wl_resource_get_user_data(resource)) };
    LSurface *lSurface { surface ? static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface))->surface() : nullptr };
    seat()->systemBellRequest(res.client(), lSurface);
}
