#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/private/GXdgWmBasePrivate.h>
#include <protocols/XdgShell/private/RXdgPopupPrivate.h>
#include <protocols/XdgShell/private/RXdgToplevelPrivate.h>
#include <protocols/XdgShell/GXdgWmBase.h>
#include <protocols/XdgShell/xdg-shell.h>

using namespace Louvre::Protocols::XdgShell;

static struct xdg_surface_interface xdg_surface_implementation =
{
    .destroy = &RXdgSurface::RXdgSurfacePrivate::destroy,
    .get_toplevel = &RXdgSurface::RXdgSurfacePrivate::get_toplevel,
    .get_popup = &RXdgSurface::RXdgSurfacePrivate::get_popup,
    .set_window_geometry = &RXdgSurface::RXdgSurfacePrivate::set_window_geometry,
    .ack_configure = &RXdgSurface::RXdgSurfacePrivate::ack_configure
};

RXdgSurface::RXdgSurface
(
    GXdgWmBase *gXdgWmBase,
    LSurface *lSurface,
    UInt32 id
)
    :LResource
    (
        gXdgWmBase->client(),
        &xdg_surface_interface,
        gXdgWmBase->version(),
        id,
        &xdg_surface_implementation,
        &RXdgSurfacePrivate::resource_destroy
    ),
    LPRIVATE_INIT_UNIQUE(RXdgSurface)
{
    imp()->gXdgWmBase = gXdgWmBase;
    imp()->lSurface = lSurface;
    xdgWmBaseGlobal()->imp()->xdgSurfaces.push_back(this);
    imp()->xdgWmBaseLink = std::prev(xdgWmBaseGlobal()->imp()->xdgSurfaces.end());
}

RXdgSurface::~RXdgSurface()
{
    if (xdgWmBaseGlobal())
        xdgWmBaseGlobal()->imp()->xdgSurfaces.erase(imp()->xdgWmBaseLink);

    if (imp()->rXdgPopup)
        imp()->rXdgPopup->imp()->rXdgSurface = nullptr;
    else if (imp()->rXdgToplevel)
        imp()->rXdgToplevel->imp()->rXdgSurface = nullptr;
}

GXdgWmBase *RXdgSurface::xdgWmBaseGlobal() const
{
    return imp()->gXdgWmBase;
}

Louvre::LSurface *RXdgSurface::surface() const
{
    return imp()->lSurface;
}

RXdgToplevel *RXdgSurface::xdgToplevelResource() const
{
    return imp()->rXdgToplevel;
}

RXdgPopup *RXdgSurface::xdgPopupResource() const
{
    return imp()->rXdgPopup;
}

bool RXdgSurface::configure(UInt32 serial) const
{
    xdg_surface_send_configure(resource(), serial);
    return true;
}
