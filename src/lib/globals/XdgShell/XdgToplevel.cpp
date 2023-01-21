#include <globals/XdgShell/XdgToplevel.h>
#include <globals/XdgShell/xdg-shell.h>

#include <private/LSurfacePrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LClientPrivate.h>

#include <LCompositor.h>
#include <LPointer.h>

#include <stdio.h>

using namespace Louvre;

void Extensions::XdgShell::Toplevel::destroy_resource(wl_resource *resource)
{
    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    delete lToplevel;
}

void Extensions::XdgShell::Toplevel::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void Extensions::XdgShell::Toplevel::set_parent (wl_client *, wl_resource *resource, wl_resource *parent)
{
    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);


    if (parent == NULL)
    {
        if (lToplevel->surface()->parent())
            lToplevel->surface()->parent()->imp()->removeChild(lToplevel->surface());
    }
    else
    {
        LSurface *lParent = ((LToplevelRole*)wl_resource_get_user_data(parent))->surface();

        if (lParent->mapped())
            lToplevel->surface()->imp()->setParent(lParent);

    }

}

void Extensions::XdgShell::Toplevel::set_title (wl_client *client, wl_resource *resource, const char *title)
{
    L_UNUSED(client);
    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->imp()->setTitle(title);
}

void Extensions::XdgShell::Toplevel::set_app_id (wl_client *client, wl_resource *resource, const char *app_id)
{
    L_UNUSED(client);
    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->imp()->setAppId(app_id);
}

void Extensions::XdgShell::Toplevel::show_window_menu (wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial, Int32 x, Int32 y)
{
    L_UNUSED(client);
    L_UNUSED(seat);
    L_UNUSED(serial);

    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->showWindowMenuRequestS(x,y);
}

void Extensions::XdgShell::Toplevel::move(wl_client *, wl_resource *resource, wl_resource *seat, UInt32 serial)
{
    L_UNUSED(seat);
    L_UNUSED(serial);

    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->startMoveRequest();
}

void Extensions::XdgShell::Toplevel::resize(wl_client *, wl_resource *resource, wl_resource *seat, UInt32 serial, UInt32 edges)
{
    L_UNUSED(seat);
    L_UNUSED(serial);

    if(edges > 10)
    {
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE, "provided value is not a valid variant of the resize_edge enum.");
        return;
    }

    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->startResizeRequest((LToplevelRole::ResizeEdge)edges);
}

void Extensions::XdgShell::Toplevel::set_max_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if(width < 0 || height < 0)
    {
        // Error enum not defined in protocol
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE, "invalid toplevel max size");
        return;
    }

    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->imp()->pendingMaxSizeS.setW(width);
    lToplevel->imp()->pendingMaxSizeS.setH(height);
    lToplevel->imp()->hasPendingMaxSize = true;
}

void Extensions::XdgShell::Toplevel::set_min_size (wl_client *client, wl_resource *resource, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if(width < 0 || height < 0)
    {
        // Error enum not defined in protocol
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE, "invalid toplevel min size");
        return;
    }
    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->imp()->pendingMinSizeS.setW(width);
    lToplevel->imp()->pendingMinSizeS.setH(height);
    lToplevel->imp()->hasPendingMinSize = true;
}

void Extensions::XdgShell::Toplevel::set_maximized (wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->setMaximizedRequest();
}

void Extensions::XdgShell::Toplevel::unset_maximized (wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->unsetMaximizedRequest();
}

void Extensions::XdgShell::Toplevel::set_fullscreen(wl_client *client, wl_resource *resource, wl_resource *output)
{
    L_UNUSED(client);

    LOutput *lOutput = nullptr;

    if(output)
        lOutput = (LOutput*)wl_resource_get_user_data(output);

    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->setFullscreenRequest(lOutput);
}

void Extensions::XdgShell::Toplevel::unset_fullscreen(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->unsetFullscreenRequest();
}

void Extensions::XdgShell::Toplevel::set_minimized(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    LToplevelRole *lToplevel= (LToplevelRole*)wl_resource_get_user_data(resource);
    lToplevel->setMinimizedRequest();
}
