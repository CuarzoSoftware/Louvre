#include <protocols/XdgShell/private/RXdgToplevelPrivate.h>

#include <protocols/Wayland/GOutput.h>
#include <protocols/XdgShell/xdg-shell.h>

#include <private/LToplevelRolePrivate.h>
#include <private/LSurfacePrivate.h>

void RXdgToplevel::RXdgToplevelPrivate::destroy_resource(wl_resource *resource)
{
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    delete rXdgToplevel;
}

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
        if (rXdgToplevel->lToplevelRole()->surface()->parent())
            rXdgToplevel->lToplevelRole()->surface()->parent()->imp()->removeChild(rXdgToplevel->lToplevelRole()->surface());
    }
    else
    {
        RXdgToplevel *rXdgParentToplevel = (RXdgToplevel*)wl_resource_get_user_data(parent);

        if (rXdgParentToplevel->lToplevelRole()->surface()->mapped())
            rXdgToplevel->lToplevelRole()->surface()->imp()->setParent(rXdgParentToplevel->lToplevelRole()->surface());
    }
}

void RXdgToplevel::RXdgToplevelPrivate::set_title(wl_client *client, wl_resource *resource, const char *title)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->lToplevelRole()->imp()->setTitle(title);
}

void RXdgToplevel::RXdgToplevelPrivate::set_app_id(wl_client *client, wl_resource *resource, const char *app_id)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->lToplevelRole()->imp()->setAppId(app_id);
}

void RXdgToplevel::RXdgToplevelPrivate::show_window_menu(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial, Int32 x, Int32 y)
{
    L_UNUSED(client);
    L_UNUSED(seat);
    L_UNUSED(serial);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->lToplevelRole()->showWindowMenuRequestS(x, y);
}

void RXdgToplevel::RXdgToplevelPrivate::move(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial)
{
    L_UNUSED(client);
    L_UNUSED(seat);
    L_UNUSED(serial);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->lToplevelRole()->startMoveRequest();
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
    rXdgToplevel->lToplevelRole()->startResizeRequest((LToplevelRole::ResizeEdge)edges);
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
    rXdgToplevel->lToplevelRole()->imp()->pendingMaxSizeS.setW(width);
    rXdgToplevel->lToplevelRole()->imp()->pendingMaxSizeS.setH(height);
    rXdgToplevel->lToplevelRole()->imp()->hasPendingMaxSize = true;
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
    rXdgToplevel->lToplevelRole()->imp()->pendingMinSizeS.setW(width);
    rXdgToplevel->lToplevelRole()->imp()->pendingMinSizeS.setH(height);
    rXdgToplevel->lToplevelRole()->imp()->hasPendingMinSize = true;
}

void RXdgToplevel::RXdgToplevelPrivate::set_maximized(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->lToplevelRole()->setMaximizedRequest();
}

void RXdgToplevel::RXdgToplevelPrivate::unset_maximized(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->lToplevelRole()->unsetMaximizedRequest();
}

void RXdgToplevel::RXdgToplevelPrivate::set_fullscreen(wl_client *client, wl_resource *resource, wl_resource *output)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);

    LOutput *lOutput = nullptr;

    if (output)
        lOutput = ((Wayland::GOutput*)wl_resource_get_user_data(output))->output();

    rXdgToplevel->lToplevelRole()->setFullscreenRequest(lOutput);
}

void RXdgToplevel::RXdgToplevelPrivate::unset_fullscreen(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->lToplevelRole()->unsetFullscreenRequest();
}

void RXdgToplevel::RXdgToplevelPrivate::set_minimized(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgToplevel *rXdgToplevel = (RXdgToplevel*)wl_resource_get_user_data(resource);
    rXdgToplevel->lToplevelRole()->setMinimizedRequest();
}
