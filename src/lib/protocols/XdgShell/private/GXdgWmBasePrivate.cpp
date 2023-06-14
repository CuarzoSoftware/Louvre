#include <protocols/XdgShell/private/GXdgWmBasePrivate.h>
#include <protocols/XdgShell/RXdgPositioner.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/Wayland/RSurface.h>
#include <LClient.h>

static struct xdg_wm_base_interface xdg_wm_base_implementation =
{
    .destroy = &GXdgWmBase::GXdgWmBasePrivate::destroy,
    .create_positioner = &GXdgWmBase::GXdgWmBasePrivate::create_positioner,
    .get_xdg_surface = &GXdgWmBase::GXdgWmBasePrivate::get_xdg_surface,
    .pong = &GXdgWmBase::GXdgWmBasePrivate::pong
};

void GXdgWmBase::GXdgWmBasePrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);
    new GXdgWmBase(client,
                   &xdg_wm_base_interface,
                   version,
                   id,
                   &xdg_wm_base_implementation,
                   &GXdgWmBase::GXdgWmBasePrivate::resource_destroy);
}

void GXdgWmBase::GXdgWmBasePrivate::resource_destroy(wl_resource *resource)
{
    GXdgWmBase *gXdgWmBase = (GXdgWmBase*)wl_resource_get_user_data(resource);
    delete gXdgWmBase;
}

void GXdgWmBase::GXdgWmBasePrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    GXdgWmBase *gXdgWmBase = (GXdgWmBase*)wl_resource_get_user_data(resource);

    if (!gXdgWmBase->xdgSurfaces().empty())
    {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES, "xdg_wm_base was destroyed before children.");
        return;
    }

    wl_resource_destroy(resource);
}

void GXdgWmBase::GXdgWmBasePrivate::create_positioner(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);
    GXdgWmBase *gXdgWmBase = (GXdgWmBase*)wl_resource_get_user_data(resource);
    new RXdgPositioner(gXdgWmBase, id);
}

void GXdgWmBase::GXdgWmBasePrivate::get_xdg_surface(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    L_UNUSED(client);
    Wayland::RSurface *rSurface = (Wayland::RSurface*)wl_resource_get_user_data(surface);

    if (rSurface->surface()->roleId() != LSurface::Undefined)
    {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_ROLE, "Given wl_surface has another role.");
        return;
    }

    GXdgWmBase *gXdgWmBase = (GXdgWmBase*)wl_resource_get_user_data(resource);
    new RXdgSurface(gXdgWmBase, rSurface->surface(), id);
}

void GXdgWmBase::GXdgWmBasePrivate::pong(wl_client *client, wl_resource *resource, UInt32 serial)
{
    L_UNUSED(client);
    GXdgWmBase *gXdgWmBase = (GXdgWmBase*)wl_resource_get_user_data(resource);
    gXdgWmBase->client()->pong(serial);
}
