#include <protocols/XdgShell/XdgWmBase.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <protocols/XdgShell/XdgSurface.h>
#include <protocols/XdgShell/XdgPositioner.h>

#include <private/LClientPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LPositionerPrivate.h>

#include <LCompositor.h>
#include <LToplevelRole.h>
#include <LPopupRole.h>

using namespace Louvre;

/* XDG_WM_BASE INTERFACES */

static struct xdg_positioner_interface xdg_positioner_implementation =
{
    .destroy = &Extensions::XdgShell::Positioner::destroy,
    .set_size = &Extensions::XdgShell::Positioner::set_size,
    .set_anchor_rect = &Extensions::XdgShell::Positioner::set_anchor_rect,
    .set_anchor = &Extensions::XdgShell::Positioner::set_anchor,
    .set_gravity = &Extensions::XdgShell::Positioner::set_gravity,
    .set_constraint_adjustment = &Extensions::XdgShell::Positioner::set_constraint_adjustment,
    .set_offset = &Extensions::XdgShell::Positioner::set_offset,
#if LOUVRE_XDG_WM_BASE_VERSION >=3
    .set_reactive = &Extensions::XdgShell::Positioner::set_reactive,
    .set_parent_size = &Extensions::XdgShell::Positioner::set_parent_size,
    .set_parent_configure = &Extensions::XdgShell::Positioner::set_parent_configure
#endif
};

static struct xdg_surface_interface xdg_surface_implementation =
{
    .destroy = &Extensions::XdgShell::Surface::destroy,
    .get_toplevel = &Extensions::XdgShell::Surface::get_toplevel,
    .get_popup = &Extensions::XdgShell::Surface::get_popup,
    .set_window_geometry = &Extensions::XdgShell::Surface::set_window_geometry,
    .ack_configure = &Extensions::XdgShell::Surface::ack_configure
};

static struct xdg_wm_base_interface xdg_wm_base_implementation =
{
    .destroy = &Extensions::XdgShell::WmBase::destroy,
    .create_positioner = &Extensions::XdgShell::WmBase::create_positioner,
    .get_xdg_surface = &Extensions::XdgShell::WmBase::get_xdg_surface,
    .pong = &Extensions::XdgShell::WmBase::pong
};

void Extensions::XdgShell::WmBase::resource_destroy(wl_resource *resource)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);
    lClient->imp()->xdgWmBaseResource = nullptr;
}

void Extensions::XdgShell::WmBase::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    // Check defunc surfaces error
    for(LSurface *surface : lClient->compositor()->surfaces())
    {
        if(surface->imp()->xdgSurfaceResource)
        {
            wl_resource_post_error(resource, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES, "xdg_wm_base was destroyed before children.");
            return;
        }
    }

    wl_resource_destroy(resource);
}

void Extensions::XdgShell::WmBase::create_positioner(wl_client *client, wl_resource *resource, UInt32 id)
{
    Int32 version = wl_resource_get_version(resource);

    // Gets the LClient from user data
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    // Creates a new LPositioner
    LPositioner *lPositioner = new LPositioner();
    lPositioner->imp()->data.client = lClient;

    // Creates the XDG_POSITIONER resource
    lPositioner->imp()->data.resource = wl_resource_create(client, &xdg_positioner_interface, version, id);

    // Binds resource to the XDG_POSITIONER interface and destructor
    wl_resource_set_implementation(lPositioner->resource(), &xdg_positioner_implementation, lPositioner, &Louvre::Extensions::XdgShell::Positioner::destroy_resource);

}

void Extensions::XdgShell::WmBase::get_xdg_surface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    // Gets the LSurface from user data
    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(surface);

    // The surface should't have a previous role
    if(lSurface->role())
    {
        wl_resource_post_error(resource,XDG_WM_BASE_ERROR_ROLE,"Given wl_surface has another role.");
        return;
    }

    // XDG_SURFACE version used by the client
    Int32 version = wl_resource_get_version(resource);

    // Creates the XDG_SURFACE resource
    lSurface->imp()->xdgSurfaceResource = wl_resource_create(client, &xdg_surface_interface, version, id);

    // Binds the resource to the XDG_SURFACE interface and destructor
    wl_resource_set_implementation(lSurface->imp()->xdgSurfaceResource, &xdg_surface_implementation, lSurface, &Louvre::Extensions::XdgShell::Surface::resource_destroy);
}

void Extensions::XdgShell::WmBase::pong(wl_client *, wl_resource *resource, UInt32 serial)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    for(LSurface *lSurface : lClient->surfaces())
    {
        if(lSurface->toplevel())
            lSurface->toplevel()->pong(serial);
        else if(lSurface->popup())
            lSurface->popup()->pong(serial);
    }
}

void Extensions::XdgShell::WmBase::bind (wl_client *client, void *data, UInt32 version, UInt32 id)
{
    LCompositor *lCompositor = (LCompositor*)data;

    LClient *lClient = nullptr;

    // Search for the client object
    for(LClient *c : lCompositor->clients())
    {
        if(c->client() == client)
        {
            lClient = c;
            break;
        }
    }

    if(!lClient)
        return;

    // Si ya tiene un xdg_wm matamos a cliente, esta librería no soporta múltiples xdg_wm por cliente
    if(lClient->imp()->xdgWmBaseResource)
    {
        wl_client_destroy(client);
        return;
    }

    // Creates the XDG_WM_BASE resource
    lClient->imp()->xdgWmBaseResource = wl_resource_create (client, &xdg_wm_base_interface, version, id);

    // Binds the resource to the XDG_WM_BASE interface and destructor
    wl_resource_set_implementation (lClient->xdgWmBaseResource(), &xdg_wm_base_implementation, lClient, &WmBase::resource_destroy);
}
