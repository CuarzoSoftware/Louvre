#include <protocols/XdgShell/private/GXdgWmBasePrivate.h>
#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols;

XdgShell::GXdgWmBase::GXdgWmBase
(
    wl_client *client,
    const wl_interface *interface,
    Int32 version,
    UInt32 id,
    const void *implementation
)
    :LResource
    (
        client,
        interface,
        version,
        id,
        implementation
    ),
    LPRIVATE_INIT_UNIQUE(GXdgWmBase)
{
    this->client()->imp()->xdgWmBaseGlobals.push_back(this);
}

GXdgWmBase::~GXdgWmBase()
{
    for (RXdgSurface *xdgSurface : xdgSurfaces())
        xdgSurface->imp()->gXdgWmBase = nullptr;

    LVectorRemoveOneUnordered(client()->imp()->xdgWmBaseGlobals, this);
}

const std::vector<RXdgSurface *> &GXdgWmBase::xdgSurfaces() const
{
    return imp()->xdgSurfaces;
}

bool XdgShell::GXdgWmBase::ping(UInt32 serial)
{
    xdg_wm_base_send_ping(resource(), serial);
    return true;
}
