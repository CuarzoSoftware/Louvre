#include <protocols/XdgShell/RXdgToplevel.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSurfacePrivate.h>
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

    m_toplevelRole.reset(compositor()->createToplevelRoleRequest(&toplevelRoleParams));
    xdgSurfaceRes->surface()->imp()->setPendingRole(toplevelRole());
}

RXdgToplevel::~RXdgToplevel()
{
    compositor()->destroyToplevelRoleRequest(toplevelRole());
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
            wl_resource_post_error(resource, 0, "Invalid xdg_toplevel parent.");
            return;
        }

        res.toplevelRole()->surface()->imp()->setParent(parentRes->toplevelRole()->surface());
    }
}

void RXdgToplevel::set_title(wl_client */*client*/, wl_resource *resource, const char *title)
{
    static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole()->imp()->setTitle(title);
}

void RXdgToplevel::set_app_id(wl_client */*client*/, wl_resource *resource, const char *app_id)
{
    static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole()->imp()->setAppId(app_id);
}

void RXdgToplevel::show_window_menu(wl_client */*client*/, wl_resource *resource, wl_resource */*seat*/, UInt32 serial, Int32 x, Int32 y)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };
    const LEvent *triggeringEvent { res.client()->findEventBySerial(serial) };

    if (triggeringEvent)
        res.toplevelRole()->showWindowMenuRequest(*triggeringEvent, x, y);
}

void RXdgToplevel::move(wl_client */*client*/, wl_resource *resource, wl_resource */*seat*/, UInt32 serial)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    if (!res.toplevelRole()->surface()->toplevel())
        return;

    const LEvent *triggererEvent { res.client()->findEventBySerial(serial) };

    if (triggererEvent)
        res.toplevelRole()->startMoveRequest(*triggererEvent);
}

void RXdgToplevel::resize(wl_client */*client*/, wl_resource *resource, wl_resource */*seat*/, UInt32 serial, UInt32 edges)
{
    if (edges > 10)
    {
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE, "provided value is not a valid variant of the resize_edge enum.");
        return;
    }

    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    if (!res.toplevelRole()->surface()->toplevel())
        return;

    const LEvent *triggererEvent { res.client()->findEventBySerial(serial) };

    if (triggererEvent)
        res.toplevelRole()->startResizeRequest(*triggererEvent, static_cast<LToplevelRole::ResizeEdge>(edges));
}

void RXdgToplevel::set_max_size(wl_client */*client*/, wl_resource *resource, Int32 width, Int32 height)
{
    if (width < 0 || height < 0)
    {
        // Error enum not defined in protocol
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE, "Invalid xdg_toplevel max size.");
        return;
    }

    auto &toplevelPriv { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole()->imp() };
    toplevelPriv.pendingMaxSize.setW(width);
    toplevelPriv.pendingMaxSize.setH(height);
    toplevelPriv.hasPendingMaxSize = true;
}

void RXdgToplevel::set_min_size(wl_client */*client*/, wl_resource *resource, Int32 width, Int32 height)
{
    if (width < 0 || height < 0)
    {
        // Error enum not defined in protocol
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE, "Invalid xdg_toplevel min size.");
        return;
    }

    auto &toplevelPriv { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource))->toplevelRole()->imp() };
    toplevelPriv.pendingMinSize.setW(width);
    toplevelPriv.pendingMinSize.setH(height);
    toplevelPriv.hasPendingMinSize = true;
}

void RXdgToplevel::set_maximized(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    // Cache until role is applied
    if (res.toplevelRole()->surface()->imp()->pending.role)
    {
        res.toplevelRole()->imp()->prevRoleRequest = LToplevelRole::Maximized;
        return;
    }

    res.toplevelRole()->setMaximizedRequest();

    if (!res.toplevelRole()->imp()->hasConfToSend)
        res.toplevelRole()->configure(res.toplevelRole()->pendingStates());
}

void RXdgToplevel::unset_maximized(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    // Cache until role is applied
    if (res.toplevelRole()->surface()->imp()->pending.role)
    {
        if (res.toplevelRole()->imp()->prevRoleRequest == LToplevelRole::Maximized)
            res.toplevelRole()->imp()->prevRoleRequest = 0;

        return;
    }

    res.toplevelRole()->unsetMaximizedRequest();

    if (!res.toplevelRole()->imp()->hasConfToSend)
        res.toplevelRole()->configure(res.toplevelRole()->pendingStates());
}

void RXdgToplevel::set_fullscreen(wl_client */*client*/, wl_resource *resource, wl_resource *wlOutput)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    LOutput *output { nullptr };

    if (wlOutput)
        output = static_cast<Wayland::GOutput*>(wl_resource_get_user_data(wlOutput))->output();

    // Cache until role is applied
    if (res.toplevelRole()->surface()->imp()->pending.role)
    {
        res.toplevelRole()->imp()->prevRoleRequest = LToplevelRole::Fullscreen;
        res.toplevelRole()->imp()->prevRoleFullscreenRequestOutput = output;
        return;
    }

    // TODO: use LWeak
    res.toplevelRole()->setFullscreenRequest(output);

    if (!res.toplevelRole()->imp()->hasConfToSend)
        res.toplevelRole()->configure(res.toplevelRole()->pendingStates());
}

void RXdgToplevel::unset_fullscreen(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    // Cache until role is applied
    if (res.toplevelRole()->surface()->imp()->pending.role)
    {
        if (res.toplevelRole()->imp()->prevRoleRequest == LToplevelRole::Fullscreen)
        {
            res.toplevelRole()->imp()->prevRoleRequest = 0;
            res.toplevelRole()->imp()->prevRoleFullscreenRequestOutput = nullptr;
        }

        return;
    }

    res.toplevelRole()->unsetFullscreenRequest();

    if (!res.toplevelRole()->imp()->hasConfToSend)
        res.toplevelRole()->configure(res.toplevelRole()->pendingStates());
}

void RXdgToplevel::set_minimized(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RXdgToplevel*>(wl_resource_get_user_data(resource)) };

    // Ignore if role not yet applied
    if (res.toplevelRole()->surface()->imp()->pending.role)
        return;

    res.toplevelRole()->setMinimizedRequest();
}

void RXdgToplevel::configure(Int32 width, Int32 height, wl_array *states) noexcept
{
    xdg_toplevel_send_configure(resource(), width, height, states);
}

void RXdgToplevel::close() noexcept
{
    xdg_toplevel_send_close(resource());
}

bool RXdgToplevel::configureBounds(Int32 width, Int32 height) noexcept
{
#if LOUVRE_XDG_WM_BASE_VERSION >= 4
    if (version() >= 4)
    {
        xdg_toplevel_send_configure_bounds(resource(), width, height);
        return true;
    }
#endif
    L_UNUSED(width);
    L_UNUSED(height);
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
