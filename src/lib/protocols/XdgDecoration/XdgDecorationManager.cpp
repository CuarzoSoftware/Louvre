#include <protocols/XdgDecoration/xdg-decoration-unstable-v1.h>
#include <protocols/XdgDecoration/XdgDecorationManager.h>
#include <protocols/XdgDecoration/XdgToplevelDecoration.h>
#include <protocols/XdgShell/xdg-shell.h>

#include <private/LClientPrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSurfacePrivate.h>

#include <LCompositor.h>
#include <LWayland.h>

using namespace Louvre;

static struct zxdg_decoration_manager_v1_interface xdg_decoration_manager_implementation
{
    .destroy = &Extensions::XdgDecoration::Manager::destroy,
    .get_toplevel_decoration = &Extensions::XdgDecoration::Manager::get_toplevel_decoration
};

static struct zxdg_toplevel_decoration_v1_interface xdg_toplevel_decoration_implementation
{
    .destroy = &Extensions::XdgDecoration::ToplevelDecoration::destroy,
    .set_mode = &Extensions::XdgDecoration::ToplevelDecoration::set_mode,
    .unset_mode = &Extensions::XdgDecoration::ToplevelDecoration::unset_mode
};

void Extensions::XdgDecoration::Manager::resource_destroy(wl_resource *resource)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);
    lClient->imp()->xdgDecorationManagerResource = nullptr;
}

void Extensions::XdgDecoration::Manager::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void Extensions::XdgDecoration::Manager::get_toplevel_decoration(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *toplevel)
{
    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(toplevel);

    Int32 version = wl_resource_get_version(resource);

    lToplevel->imp()->xdgDecoration = wl_resource_create(client, &zxdg_toplevel_decoration_v1_interface, version, id);

    wl_resource_set_implementation(lToplevel->imp()->xdgDecoration, &xdg_toplevel_decoration_implementation, lToplevel, &Louvre::Extensions::XdgDecoration::ToplevelDecoration::resource_destroy);

    lToplevel->imp()->lastDecorationModeConfigureSerial = LWayland::nextSerial();
    zxdg_toplevel_decoration_v1_send_configure(lToplevel->imp()->xdgDecoration, lToplevel->imp()->pendingDecorationMode);
    xdg_surface_send_configure(lToplevel->surface()->imp()->xdgSurfaceResource, lToplevel->imp()->lastDecorationModeConfigureSerial);
}

void Extensions::XdgDecoration::Manager::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
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

    lClient->imp()->xdgDecorationManagerResource = wl_resource_create (client, &zxdg_decoration_manager_v1_interface, version, id);

    wl_resource_set_implementation(lClient->xdgDecorationManagerResource(), &xdg_decoration_manager_implementation, lClient, &Extensions::XdgDecoration::Manager::resource_destroy);
}
