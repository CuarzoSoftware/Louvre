#include <protocols/ForeignToplevelList/RForeignToplevelHandle.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LFactory.h>
#include <LCompositor.h>
#include <LClient.h>

using namespace Louvre::Protocols::XdgShell;

static const struct xdg_toplevel_interface imp
{
    .destroy = &RXdgToplevel::destroy,
    .set_parent = &RXdgToplevel::set_parent,
    .set_title = &RXdgToplevel::set_title,
    .set_app_id = &RXdgToplevel::set_app_id,
    .show_window_menu = &RXdgToplevel::show_window_menu,
    .move = &RXdgToplevel::move,
    .resize = &RXdgToplevel::resize,
    .set_max_size = &RXdgToplevel::set_max_size,
    .set_min_size = &RXdgToplevel::set_min_size,
    .set_maximized = &RXdgToplevel::set_maximized,
    .unset_maximized = &RXdgToplevel::unset_maximized,
    .set_fullscreen = &RXdgToplevel::set_fullscreen,
    .unset_fullscreen = &RXdgToplevel::unset_fullscreen,
    .set_minimized = &RXdgToplevel::set_minimized
};

RXdgToplevel::RXdgToplevel
(
    RXdgSurface *xdgSurfaceRes,
    UInt32 id
)
:LResource
    (
        xdgSurfaceRes->client(),
        &xdg_toplevel_interface,
        xdgSurfaceRes->version(),
        id,
        &imp
    ),
    m_xdgSurfaceRes(xdgSurfaceRes)
{
    xdgSurfaceRes->m_xdgToplevelRes.reset(this);

    LToplevelRole::Params toplevelRoleParams
    {
        this,
        xdgSurfaceRes->surface()
    };

    m_toplevelRole.reset(LFactory::createObject<LToplevelRole>(&toplevelRoleParams));
    xdgSurfaceRes->surface()->imp()->notifyRoleChange();
}

RXdgToplevel::~RXdgToplevel()
{
    compositor()->onAnticipatedObjectDestruction(toplevelRole());

    while (!toplevelRole()->m_foreignToplevelHandles.empty())
        toplevelRole()->m_foreignToplevelHandles.back()->closed();

    toplevelRole()->surface()->imp()->setMapped(false);
    toplevelRole()->surface()->imp()->setRole(nullptr);
    toplevelRole()->surface()->imp()->notifyRoleChange();
    m_toplevelRole.reset();
}

void RXdgToplevel::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RXdgToplevel::set_parent(wl_client */*client*/, wl_resource *resource, wl_resource *parent)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    if (parent == NULL)
    {
        if (res.toplevelRole()->surface()->parent())
            res.toplevelRole()->surface()->imp()->setParent(nullptr);
    }
    else
    {
        auto *parentRes { static_cast<RXdgToplevel*>(wl_resource_get_user_data(parent)) };

        // Setting a parent surface that is not mapped = setting null parent
        if (!parentRes->toplevelRole()->surface()->mapped())
        {
            parentRes->toplevelRole()->surface()->imp()->setParent(nullptr);
            return;
        }

        if (res.toplevelRole()->surface()->imp()->isInChildrenOrPendingChildren(parentRes->toplevelRole()->surface()))
        {
            res.postError(XDG_TOPLEVEL_ERROR_INVALID_PARENT, "Invalid xdg_toplevel parent.");
            return;
        }

        res.toplevelRole()->surface()->imp()->setParent(parentRes->toplevelRole()->surface());
    }
}

void RXdgToplevel::set_title(wl_client */*client*/, wl_resource *resource, const char *title)
{
    static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole()->setTitle(title);
}

void RXdgToplevel::set_app_id(wl_client */*client*/, wl_resource *resource, const char *app_id)
{
    static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole()->setAppId(app_id);
}

void RXdgToplevel::show_window_menu(wl_client */*client*/, wl_resource *resource, wl_resource */*seat*/, UInt32 serial, Int32 x, Int32 y)
{
    auto &toplevel { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole() };
    const LEvent *triggeringEvent { toplevel.client()->findEventBySerial(serial) };

    if (triggeringEvent && toplevel.capabilities().check(LToplevelRole::WindowMenuCap))
        toplevel.showWindowMenuRequest(*triggeringEvent, x, y);
}

void RXdgToplevel::move(wl_client */*client*/, wl_resource *resource, wl_resource */*seat*/, UInt32 serial)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    if (!res.toplevelRole()->surface()->mapped())
        return;

    const LEvent *triggererEvent { res.client()->findEventBySerial(serial) };

    if (triggererEvent)
        res.toplevelRole()->startMoveRequest(*triggererEvent);
}

void RXdgToplevel::resize(wl_client */*client*/, wl_resource *resource, wl_resource */*seat*/, UInt32 serial, UInt32 edges)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    if (edges > 10)
    {
        res.postError(
            XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE,
            "Provided value is not a valid variant of the resize_edge enum.");
        return;
    }

    if (!res.toplevelRole()->surface()->mapped())
        return;

    const LEvent *triggererEvent { res.client()->findEventBySerial(serial) };

    if (triggererEvent)
        res.toplevelRole()->startResizeRequest(*triggererEvent, edges);
}

void RXdgToplevel::set_max_size(wl_client */*client*/, wl_resource *resource, Int32 width, Int32 height)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    if (width < 0 || height < 0)
    {
        res.postError(XDG_TOPLEVEL_ERROR_INVALID_SIZE, "Invalid xdg_toplevel max size ({}, {}).", width, height);
        return;
    }

    res.toplevelRole()->pendingAtoms().maxSize.setW(width);
    res.toplevelRole()->pendingAtoms().maxSize.setH(height);
}

void RXdgToplevel::set_min_size(wl_client */*client*/, wl_resource *resource, Int32 width, Int32 height)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    if (width < 0 || height < 0)
    {
        res.postError(XDG_TOPLEVEL_ERROR_INVALID_SIZE, "Invalid xdg_toplevel min size ({}, {}).", width, height);
        return;
    }

    res.toplevelRole()->pendingAtoms().minSize.setW(width);
    res.toplevelRole()->pendingAtoms().minSize.setH(height);
}

void RXdgToplevel::set_maximized(wl_client */*client*/, wl_resource *resource)
{
    auto &toplevel { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole() };

    if (toplevel.m_flags.check(LToplevelRole::HasPendingInitialConf))
    {
        toplevel.m_requestedStateBeforeConf = LToplevelRole::Maximized;
        return;
    }

    if (toplevel.capabilities().check(LToplevelRole::MaximizeCap))
        toplevel.setMaximizedRequest();

    if (!toplevel.m_flags.check(LToplevelRole::HasSizeOrStateToSend)
        && (toplevel.capabilities().check(LToplevelRole::MaximizeCap) || toplevel.resource()->version() < 5))
        toplevel.configureState(toplevel.pendingConfiguration().state);
}

void RXdgToplevel::unset_maximized(wl_client */*client*/, wl_resource *resource)
{
    auto &toplevel { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole() };

    if (toplevel.m_flags.check(LToplevelRole::HasPendingInitialConf))
    {
        if (toplevel.m_requestedStateBeforeConf == LToplevelRole::Maximized)
            toplevel.m_requestedStateBeforeConf = LToplevelRole::NoState;
        return;
    }

    if (toplevel.capabilities().check(LToplevelRole::MaximizeCap))
        toplevel.unsetMaximizedRequest();

    if (!toplevel.m_flags.check(LToplevelRole::HasSizeOrStateToSend)
        && (toplevel.capabilities().check(LToplevelRole::MaximizeCap) || toplevel.resource()->version() < 5))
        toplevel.configureState(toplevel.pendingConfiguration().state);
}

void RXdgToplevel::set_fullscreen(wl_client */*client*/, wl_resource *resource, wl_resource *wlOutput)
{
    LOutput *output { nullptr };

    if (wlOutput)
        output = static_cast<Wayland::GOutput*>(wl_resource_get_user_data(wlOutput))->output();

    auto &toplevel { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole() };

    if (toplevel.m_flags.check(LToplevelRole::HasPendingInitialConf))
    {
        toplevel.m_requestedStateBeforeConf = LToplevelRole::Fullscreen;
        toplevel.m_fullscreenOutputBeforeConf.reset(output);
        return;
    }

    if (toplevel.capabilities().check(LToplevelRole::FullscreenCap))
        toplevel.setFullscreenRequest(output);

    if (!toplevel.m_flags.check(LToplevelRole::HasSizeOrStateToSend)
        && (toplevel.capabilities().check(LToplevelRole::FullscreenCap) || toplevel.resource()->version() < 5))
        toplevel.configureState(toplevel.pendingConfiguration().state);
}

void RXdgToplevel::unset_fullscreen(wl_client */*client*/, wl_resource *resource)
{    
    auto &toplevel { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole() };

    if (toplevel.m_flags.check(LToplevelRole::HasPendingInitialConf))
    {
        if (toplevel.m_requestedStateBeforeConf == LToplevelRole::Fullscreen)
        {
            toplevel.m_requestedStateBeforeConf = LToplevelRole::NoState;
            toplevel.m_fullscreenOutputBeforeConf.reset(nullptr);
        }
        return;
    }

    if (toplevel.capabilities().check(LToplevelRole::FullscreenCap))
        toplevel.unsetFullscreenRequest();

    if (!toplevel.m_flags.check(LToplevelRole::HasSizeOrStateToSend)
        && (toplevel.capabilities().check(LToplevelRole::FullscreenCap) || toplevel.resource()->version() < 5))
        toplevel.configureState(toplevel.pendingConfiguration().state);
}

void RXdgToplevel::set_minimized(wl_client */*client*/, wl_resource *resource)
{
    auto &toplevel { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole() };

    if (toplevel.m_flags.check(LToplevelRole::HasPendingInitialConf))
        return;

    if (toplevel.capabilities().check(LToplevelRole::MinimizeCap))
        toplevel.setMinimizedRequest();
}

void RXdgToplevel::configure(const LSize &size, wl_array *states) noexcept
{
    xdg_toplevel_send_configure(resource(), size.w(), size.h(), states);
}

void RXdgToplevel::close() noexcept
{
    xdg_toplevel_send_close(resource());
}

bool RXdgToplevel::configureBounds(const LSize &bounds) noexcept
{
#if LOUVRE_XDG_WM_BASE_VERSION >= 4
    if (version() >= 4)
    {
        xdg_toplevel_send_configure_bounds(resource(), bounds.w(), bounds.h());
        return true;
    }
#endif
    L_UNUSED(bounds);
    return false;
}

bool RXdgToplevel::wmCapabilities(wl_array *capabilities) noexcept
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
