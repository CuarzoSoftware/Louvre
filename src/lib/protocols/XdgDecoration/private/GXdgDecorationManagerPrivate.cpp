#include <protocols/XdgDecoration/private/GXdgDecorationManagerPrivate.h>
#include <protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <protocols/XdgDecoration/xdg-decoration-unstable-v1.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSurfacePrivate.h>

static struct zxdg_decoration_manager_v1_interface xdg_decoration_manager_implementation
{
    .destroy = &GXdgDecorationManager::GXdgDecorationManagerPrivate::destroy,
    .get_toplevel_decoration = &GXdgDecorationManager::GXdgDecorationManagerPrivate::get_toplevel_decoration
};

void GXdgDecorationManager::GXdgDecorationManagerPrivate::bind(wl_client *client,
                                                               void *data,
                                                               UInt32 version,
                                                               UInt32 id)
{
    L_UNUSED(data);
    new GXdgDecorationManager(client,
                              &zxdg_decoration_manager_v1_interface,
                              version,
                              id,
                              &xdg_decoration_manager_implementation);
}

void GXdgDecorationManager::GXdgDecorationManagerPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void GXdgDecorationManager
    ::GXdgDecorationManagerPrivate::get_toplevel_decoration(wl_client *client,
                                                            wl_resource *resource,
                                                            UInt32 id,
                                                            wl_resource *toplevel)
{
    L_UNUSED(client);
    XdgShell::RXdgToplevel *rXdgToplevelRole = (XdgShell::RXdgToplevel*)wl_resource_get_user_data(toplevel);

    // Prevent client from creating more than one resource for a given Toplevel
    if (rXdgToplevelRole->toplevelRole()->imp()->xdgDecoration)
    {
        wl_resource_post_error(resource, 0, "Multiple XDG Toplevel Decorations for a Toplevel not supported.");
        return;
    }

    if (rXdgToplevelRole->xdgSurfaceResource()->surface()->imp()->hasBufferOrPendingBuffer())
    {
        wl_resource_post_error(resource, ZXDG_TOPLEVEL_DECORATION_V1_ERROR_UNCONFIGURED_BUFFER,
                               "Given Toplevel already has a buffer attached.");
        return;
    }

    GXdgDecorationManager *gXdgDecorationManager = (GXdgDecorationManager*)wl_resource_get_user_data(resource);
    new RXdgToplevelDecoration(gXdgDecorationManager, rXdgToplevelRole->toplevelRole(), id);
}
