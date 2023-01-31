#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSurfacePrivate.h>

#include <protocols/Wayland/Compositor.h>
#include <protocols/Wayland/Region.h>
#include <protocols/Wayland/Surface.h>

#include <unistd.h>

using namespace std;
using namespace Louvre;

struct wl_surface_interface surface_implementation =
{
    .destroy                = &Globals::Surface::destroy,
    .attach                 = &Globals::Surface::attach,
    .damage                 = &Globals::Surface::damage,
    .frame                  = &Globals::Surface::frame,
    .set_opaque_region      = &Globals::Surface::set_opaque_region,
    .set_input_region       = &Globals::Surface::set_input_region,
    .commit                 = &Globals::Surface::commit,

#if LOUVRE_COMPOSITOR_VERSION >= 2
    .set_buffer_transform   = &Globals::Surface::set_buffer_transform,
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 3
    .set_buffer_scale       = &Globals::Surface::set_buffer_scale,
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 4
    .damage_buffer          = &Globals::Surface::damage_buffer,
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 5
    .offset                 = &Globals::Surface::offset
#endif

};

struct wl_region_interface region_implementation =
{
    .destroy = &Globals::Region::destroy,
    .add = &Globals::Region::add,
    .subtract = &Globals::Region::subtract
};

struct wl_compositor_interface compositor_implementation =
{
    .create_surface = &Globals::Compositor::create_surface,
    .create_region = &Globals::Compositor::create_region
};

void Globals::Compositor::create_surface(wl_client *client, wl_resource *resource, UInt32 id)
{
    Int32 version = wl_resource_get_version(resource);

    // New surface resource
    wl_resource *surface = wl_resource_create(client, &wl_surface_interface, version, id);

    // Find client
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    // Create surface
    LSurface::Params params;
    params.surface = surface;
    params.client = lClient;
    LSurface *lSurface = lClient->compositor()->createSurfaceRequest(&params);

    // Append surface
    lClient->imp()->surfaces.push_back(lSurface);
    lSurface->imp()->clientLink = std::prev(lClient->imp()->surfaces.end());
    lClient->compositor()->imp()->surfaces.push_back(lSurface);
    lSurface->imp()->compositorLink = std::prev(lClient->compositor()->imp()->surfaces.end());

    // Implement surface
    wl_resource_set_implementation(surface, &surface_implementation, lSurface, &Surface::resource_destroy);
}

void Globals::Compositor::create_region (wl_client *client, wl_resource *resource, UInt32 id)
{
    // New region
    wl_resource *region = wl_resource_create(client, &wl_region_interface, wl_resource_get_version(resource), id);

    // Create region
    LRegion *lRegion = new LRegion();

    // Implement region
    wl_resource_set_implementation(region, &region_implementation, lRegion, &Region::resource_destroy);
}

void Globals::Compositor::resource_destroy(wl_resource *resource)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);
    lClient->imp()->compositorResource = nullptr;
}

void Globals::Compositor::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
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

    lClient->imp()->compositorResource = wl_resource_create(client, &wl_compositor_interface, version, id);
    wl_resource_set_implementation(lClient->imp()->compositorResource, &compositor_implementation, lClient, &Compositor::resource_destroy);
}
