#include <globals/XdgShell/XdgSurface.h>
#include <globals/XdgShell/XdgToplevel.h>
#include <globals/XdgShell/XdgPopup.h>
#include <globals/XdgShell/xdg-shell.h>

#include <private/LToplevelRolePrivate.h>
#include <private/LPopupRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LPositionerPrivate.h>

#include <LCompositor.h>


using namespace Louvre;

static struct xdg_toplevel_interface xdg_toplevel_implementation =
{
    .destroy = &Extensions::XdgShell::Toplevel::destroy,
    .set_parent = &Extensions::XdgShell::Toplevel::set_parent,
    .set_title = &Extensions::XdgShell::Toplevel::set_title,
    .set_app_id = &Extensions::XdgShell::Toplevel::set_app_id,
    .show_window_menu = &Extensions::XdgShell::Toplevel::show_window_menu,
    .move = &Extensions::XdgShell::Toplevel::move,
    .resize = &Extensions::XdgShell::Toplevel::resize,
    .set_max_size = &Extensions::XdgShell::Toplevel::set_max_size,
    .set_min_size = &Extensions::XdgShell::Toplevel::set_min_size,
    .set_maximized = &Extensions::XdgShell::Toplevel::set_maximized,
    .unset_maximized = &Extensions::XdgShell::Toplevel::unset_maximized,
    .set_fullscreen = &Extensions::XdgShell::Toplevel::set_fullscreen,
    .unset_fullscreen = &Extensions::XdgShell::Toplevel::unset_fullscreen,
    .set_minimized = &Extensions::XdgShell::Toplevel::set_minimized
};

static struct xdg_popup_interface xdg_popup_implementation =
{
    .destroy = &Extensions::XdgShell::Popup::destroy,
    .grab = &Extensions::XdgShell::Popup::grab,

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    .reposition = &Extensions::XdgShell::Popup::reposition
#endif

};

// xdg surface
void Extensions::XdgShell::Surface::resource_destroy(wl_resource *resource)
{
    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);

    if(lSurface)
        lSurface->imp()->xdgSurfaceResource = nullptr;
}

void Extensions::XdgShell::Surface::destroy (wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);

    if (lSurface && lSurface->role())
    {
        wl_resource_post_error(resource, 0, "xdg_surface must be destroyed after its specific role");
        return;
    }


    wl_resource_destroy(resource);
}

void Extensions::XdgShell::Surface::get_toplevel(wl_client *client, wl_resource *resource, UInt32 id)
{
    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);

    if (lSurface->imp()->pending.buffer || lSurface->imp()->current.buffer)
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
        return;
    }

    if (lSurface->role() )
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "xdg_surface already has a role object.");
        return;
    }

    wl_resource *toplevel = wl_resource_create(client, &xdg_toplevel_interface, wl_resource_get_version(resource), id);

    LToplevelRole::Params toplevelRoleParams;
    toplevelRoleParams.toplevel = toplevel;
    toplevelRoleParams.surface = lSurface;
    lSurface->imp()->setPendingRole(lSurface->compositor()->createToplevelRoleRequest(&toplevelRoleParams));
    wl_resource_set_implementation(toplevel, &xdg_toplevel_implementation, lSurface->imp()->pending.role, &Extensions::XdgShell::Toplevel::destroy_resource);

}
void Extensions::XdgShell::Surface::get_popup(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *parent, wl_resource *positioner)
{

    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);

    if (lSurface->imp()->pending.buffer != nullptr || lSurface->imp()->current.buffer != nullptr)
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
        return;
    }

    if (lSurface->role())
    {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "xdg_surface already has a role object.");
        return;
    }

    if (parent)
    {
        lSurface->imp()->pendingParent = (LSurface*)wl_resource_get_user_data(parent);
    }
    else
    {
        lSurface->imp()->pendingParent = nullptr;
    }


    LPositioner *lPositioner = (LPositioner*)wl_resource_get_user_data(positioner);

    if (lPositioner->sizeS().area() <= 0 || lPositioner->anchorRectS().area() <= 0)
    {
        wl_resource_post_error(
                    positioner,
                    XDG_WM_BASE_ERROR_INVALID_POSITIONER,
                    "xdg_surface.get_popup with invalid positioner (size: %dx%d, anchorRect: %dx%d)",
                    lPositioner->sizeS().w(),lPositioner->sizeS().h(),lPositioner->anchorRectS().w(),lPositioner->anchorRectS().h());
        return;
    }


    wl_resource *popup = wl_resource_create(client, &xdg_popup_interface, wl_resource_get_version(resource), id);

    LPopupRole::Params popupRoleParams;
    popupRoleParams.popup = popup;
    popupRoleParams.surface = lSurface;
    popupRoleParams.positioner = lPositioner;

    LPopupRole *lPopup = lSurface->compositor()->createPopupRoleRequest(&popupRoleParams);
    wl_resource_set_implementation(popup, &xdg_popup_implementation, lPopup, &Extensions::XdgShell::Popup::destroy_resource);

    lSurface->imp()->setPendingRole(lPopup);


}
void Extensions::XdgShell::Surface::set_window_geometry(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    LSurface *surface = (LSurface*)wl_resource_get_user_data(resource);

    if (width <= 0 || height <= 0)
    {

        wl_resource_post_error(
                    resource,
                    0,
                    "Invalid window geometry size.");

        return;
    }


    if(surface->toplevel())
    {
        surface->toplevel()->imp()->windowGeometrySet = true;
        surface->toplevel()->imp()->pendingWindowGeometryS = LRect(x, y, width, height);
        surface->toplevel()->imp()->hasPendingWindowGeometry = true;
    }
    else if(surface->popup())
    {
        surface->popup()->imp()->windowGeometrySet = true;
        surface->popup()->imp()->pendingWindowGeometryS = LRect(x, y, width, height);
        surface->popup()->imp()->hasPendingWindowGeometry = true;
    }
    else
    {
        wl_resource_post_error(
                    surface->xdgSurfaceResource(),
                    XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
                    "wl_surface does not have a role yet.");
    }
}
void Extensions::XdgShell::Surface::ack_configure(wl_client *client, wl_resource *resource, UInt32 serial)
{
    L_UNUSED(client);

    LSurface *surface = (LSurface*)wl_resource_get_user_data(resource);

    if(surface->roleId() == LSurface::Role::Toplevel)
    {
        LToplevelRole *toplevel = surface->toplevel();


        while(!toplevel->imp()->sentConfs.empty())
        {
            if(toplevel->imp()->sentConfs.front().serial == serial)
            {
                toplevel->imp()->currentConf = toplevel->imp()->sentConfs.front();
                toplevel->imp()->sentConfs.pop_front();
                break;
            }

            toplevel->imp()->sentConfs.pop_front();
        }


        if(toplevel->imp()->xdgDecoration && toplevel->imp()->pendingDecorationMode != 0 && toplevel->imp()->lastDecorationModeConfigureSerial == serial)
        {
            toplevel->imp()->decorationMode = (LToplevelRole::DecorationMode)toplevel->imp()->pendingDecorationMode;
            toplevel->decorationModeChanged();
            toplevel->imp()->pendingDecorationMode = 0;
            return;
        }
    }
    else if(surface->roleId() == LSurface::Role::Popup)
    {

    }
    else
    {
        wl_resource_post_error(
                    surface->xdgSurfaceResource(),
                    XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
                    "wl_surface does not have a role yet.");
    }

}

