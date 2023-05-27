#include <protocols/XdgShell/private/RXdgToplevelPrivate.h>
#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>

#include <protocols/XdgShell/xdg-shell.h>

#include <private/LToplevelRolePrivate.h>
#include <private/LSurfacePrivate.h>

#include <LCompositor.h>
#include <LWayland.h>

static struct xdg_toplevel_interface xdg_toplevel_implementation =
{
    .destroy = &RXdgToplevel::RXdgToplevelPrivate::destroy,
    .set_parent = &RXdgToplevel::RXdgToplevelPrivate::set_parent,
    .set_title = &RXdgToplevel::RXdgToplevelPrivate::set_title,
    .set_app_id = &RXdgToplevel::RXdgToplevelPrivate::set_app_id,
    .show_window_menu = &RXdgToplevel::RXdgToplevelPrivate::show_window_menu,
    .move = &RXdgToplevel::RXdgToplevelPrivate::move,
    .resize = &RXdgToplevel::RXdgToplevelPrivate::resize,
    .set_max_size = &RXdgToplevel::RXdgToplevelPrivate::set_max_size,
    .set_min_size = &RXdgToplevel::RXdgToplevelPrivate::set_min_size,
    .set_maximized = &RXdgToplevel::RXdgToplevelPrivate::set_maximized,
    .unset_maximized = &RXdgToplevel::RXdgToplevelPrivate::unset_maximized,
    .set_fullscreen = &RXdgToplevel::RXdgToplevelPrivate::set_fullscreen,
    .unset_fullscreen = &RXdgToplevel::RXdgToplevelPrivate::unset_fullscreen,
    .set_minimized = &RXdgToplevel::RXdgToplevelPrivate::set_minimized
};

RXdgToplevel::RXdgToplevel
(
    RXdgSurface *rXdgSurface,
    UInt32 id
)
:LResource
    (
        rXdgSurface->client(),
        &xdg_toplevel_interface,
        rXdgSurface->version(),
        id,
        &xdg_toplevel_implementation,
        &RXdgToplevelPrivate::destroy_resource
    )
{
    m_imp = new RXdgToplevelPrivate();
    imp()->rXdgSurface = rXdgSurface;
    rXdgSurface->imp()->rXdgToplevel = this;

    LToplevelRole::Params toplevelRoleParams;
    toplevelRoleParams.toplevel = this;
    toplevelRoleParams.surface = rXdgSurface->lSurface();
    imp()->lToplevelRole = compositor()->createToplevelRoleRequest(&toplevelRoleParams);
    rXdgSurface->lSurface()->imp()->setPendingRole(imp()->lToplevelRole);
}

RXdgToplevel::~RXdgToplevel()
{
    if (rXdgSurface())
        rXdgSurface()->imp()->rXdgToplevel = nullptr;

    delete imp()->lToplevelRole;
    delete m_imp;
}

RXdgSurface *RXdgToplevel::rXdgSurface() const
{
    return imp()->rXdgSurface;
}

LToplevelRole *RXdgToplevel::lToplevelRole() const
{
    return imp()->lToplevelRole;
}

bool RXdgToplevel::configure(Int32 width, Int32 height, wl_array *states) const
{
    xdg_toplevel_send_configure(resource(), width, height, states);
    return true;
}

bool RXdgToplevel::close() const
{
    xdg_toplevel_send_close(resource());
    return true;
}

bool RXdgToplevel::configure_bounds(Int32 width, Int32 height) const
{
#if LOUVRE_XDG_WM_BASE_VERSION >= 4
    if (version() >= 4)
    {
        xdg_toplevel_send_configure_bounds(resource(), width, height);
        return true;
    }
#endif
    return false;
}

bool RXdgToplevel::wm_capabilities(wl_array *capabilities) const
{
#if LOUVRE_XDG_WM_BASE_VERSION >= 5
    if (version() >= 5)
    {
        xdg_toplevel_send_wm_capabilities(resource(), capabilities);
        return true;
    }
#endif
    L_UNUSED(capabilities);
    return false;
}
