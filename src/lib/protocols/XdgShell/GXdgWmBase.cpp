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
    const void *implementation,
    wl_resource_destroy_func_t destroy
)
    :LResource
    (
        client,
        interface,
        version,
        id,
        implementation,
        destroy
    )
{
    m_imp = new GXdgWmBasePrivate();
    this->client()->imp()->xdgWmBaseGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->xdgWmBaseGlobals.end());
}

GXdgWmBase::~GXdgWmBase()
{
    for (RXdgSurface *xdgSurface : xdgSurfaces())
        xdgSurface->imp()->gXdgWmBase = nullptr;

    client()->imp()->xdgWmBaseGlobals.erase(imp()->clientLink);
    delete m_imp;
}

const list<RXdgSurface *> &GXdgWmBase::xdgSurfaces() const
{
    return imp()->xdgSurfaces;
}

void XdgShell::GXdgWmBase::ping(UInt32 serial) const
{
    xdg_wm_base_send_ping(resource(), serial);
}
