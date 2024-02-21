#include <protocols/XdgShell/private/RXdgToplevelPrivate.h>
#include <protocols/Wayland/GOutput.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSurfacePrivate.h>

void RXdgToplevel::RXdgToplevelPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RXdgToplevel::RXdgToplevelPrivate::set_parent(wl_client *client, wl_resource *resource, wl_resource *parent)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);

    if (parent == NULL)
    {
        if (rXdgToplevel->toplevelRole()->surface()->parent())
            rXdgToplevel->toplevelRole()->surface()->imp()->setParent(nullptr);
    }
    else
    {
        RXdgToplevel *rXdgParentToplevel = (RXdgToplevel*)wl_resource_get_user_data(parent);

        // Setting a parent surface that is not mapped = setting null parent
        if (!rXdgParentToplevel->toplevelRole()->surface()->mapped())
        {
            rXdgToplevel->toplevelRole()->surface()->imp()->setParent(nullptr);
            return;
        }

        if (rXdgToplevel->toplevelRole()->surface()->imp()->isInChildrenOrPendingChildren(
                rXdgParentToplevel->toplevelRole()->surface()))
        {
            wl_resource_post_error(resource, 0, "Invalid xdg_toplevel parent.");
            return;
        }

        rXdgToplevel->toplevelRole()->surface()->imp()->setParent(rXdgParentToplevel->toplevelRole()->surface());
    }
}

void RXdgToplevel::RXdgToplevelPrivate::set_title(wl_client *client, wl_resource *resource, const char *title)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->toplevelRole()->imp()->setTitle(title);
}

void RXdgToplevel::RXdgToplevelPrivate::set_app_id(wl_client *client, wl_resource *resource, const char *app_id)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->toplevelRole()->imp()->setAppId(app_id);
}

void RXdgToplevel::RXdgToplevelPrivate::show_window_menu(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial, Int32 x, Int32 y)
{
    L_UNUSED(client);
    L_UNUSED(seat);
    L_UNUSED(serial);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->toplevelRole()->showWindowMenuRequest(x, y);
}

void RXdgToplevel::RXdgToplevelPrivate::move(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial)
{
    L_UNUSED(client);
    L_UNUSED(seat);
    L_UNUSED(serial);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);

    if (!rXdgToplevel->toplevelRole()->surface()->toplevel())
        return;

    rXdgToplevel->toplevelRole()->startMoveRequest();
}

void RXdgToplevel::RXdgToplevelPrivate::resize(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial, UInt32 edges)
{
    L_UNUSED(client);
    L_UNUSED(seat);
    L_UNUSED(serial);

    if (edges > 10)
    {
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE, "provided value is not a valid variant of the resize_edge enum.");
        return;
    }

    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);

    if (!rXdgToplevel->toplevelRole()->surface()->toplevel())
        return;

    rXdgToplevel->toplevelRole()->startResizeRequest((LToplevelRole::ResizeEdge)edges);
}

void RXdgToplevel::RXdgToplevelPrivate::set_max_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if (width < 0 || height < 0)
    {
        // Error enum not defined in protocol
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE, "Invalid xdg_toplevel max size.");
        return;
    }

    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->toplevelRole()->imp()->pendingMaxSize.setW(width);
    rXdgToplevel->toplevelRole()->imp()->pendingMaxSize.setH(height);
    rXdgToplevel->toplevelRole()->imp()->hasPendingMaxSize = true;
}

void RXdgToplevel::RXdgToplevelPrivate::set_min_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if (width < 0 || height < 0)
    {
        // Error enum not defined in protocol
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE, "Invalid xdg_toplevel min size.");
        return;
    }

    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->toplevelRole()->imp()->pendingMinSize.setW(width);
    rXdgToplevel->toplevelRole()->imp()->pendingMinSize.setH(height);
    rXdgToplevel->toplevelRole()->imp()->hasPendingMinSize = true;
}

void RXdgToplevel::RXdgToplevelPrivate::set_maximized(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    LToplevelRole *lToplevel = rXdgToplevel->toplevelRole();

    // Cache until role is applied
    if (lToplevel->surface()->imp()->pending.role)
    {
        lToplevel->imp()->prevRoleRequest = LToplevelRole::Maximized;
        return;
    }

    rXdgToplevel->toplevelRole()->setMaximizedRequest();

    if (!lToplevel->imp()->hasConfToSend)
        lToplevel->configure(lToplevel->pendingStates());
}

void RXdgToplevel::RXdgToplevelPrivate::unset_maximized(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    LToplevelRole *lToplevel = rXdgToplevel->toplevelRole();

    // Cache until role is applied
    if (lToplevel->surface()->imp()->pending.role)
    {
        if (lToplevel->imp()->prevRoleRequest == LToplevelRole::Maximized)
            lToplevel->imp()->prevRoleRequest = 0;

        return;
    }

    rXdgToplevel->toplevelRole()->unsetMaximizedRequest();

    if (!lToplevel->imp()->hasConfToSend)
        lToplevel->configure(lToplevel->pendingStates());
}

void RXdgToplevel::RXdgToplevelPrivate::set_fullscreen(wl_client *client, wl_resource *resource, wl_resource *output)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    LToplevelRole *lToplevel = rXdgToplevel->toplevelRole();

    LOutput *lOutput = nullptr;

    if (output)
        lOutput = ((Wayland::GOutput*)wl_resource_get_user_data(output))->output();

    // Cache until role is applied
    if (lToplevel->surface()->imp()->pending.role)
    {
        lToplevel->imp()->prevRoleRequest = LToplevelRole::Fullscreen;
        lToplevel->imp()->prevRoleFullscreenRequestOutput = lOutput;
        return;
    }

    rXdgToplevel->toplevelRole()->setFullscreenRequest(lOutput);

    if (!lToplevel->imp()->hasConfToSend)
        lToplevel->configure(lToplevel->pendingStates());
}

void RXdgToplevel::RXdgToplevelPrivate::unset_fullscreen(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    LToplevelRole *lToplevel = rXdgToplevel->toplevelRole();

    // Cache until role is applied
    if (lToplevel->surface()->imp()->pending.role)
    {
        if (lToplevel->imp()->prevRoleRequest == LToplevelRole::Fullscreen)
        {
            lToplevel->imp()->prevRoleRequest = 0;
            lToplevel->imp()->prevRoleFullscreenRequestOutput = nullptr;
        }

        return;
    }

    rXdgToplevel->toplevelRole()->unsetFullscreenRequest();

    if (!lToplevel->imp()->hasConfToSend)
        lToplevel->configure(lToplevel->pendingStates());
}

void RXdgToplevel::RXdgToplevelPrivate::set_minimized(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);

    // Ignore if role not yet applied
    if (rXdgToplevel->toplevelRole()->surface()->imp()->pending.role)
        return;

    rXdgToplevel->toplevelRole()->setMinimizedRequest();
}
