#include <protocols/Wayland/private/CompositorGlobalPrivate.h>
#include <protocols/Wayland/SurfaceResource.h>
#include <protocols/Wayland/RegionResource.h>


void CompositorGlobal::CompositorGlobalPrivate::create_surface(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);
    CompositorGlobal *lCompositorGlobal = (CompositorGlobal*)wl_resource_get_user_data(resource);
    new SurfaceResource(lCompositorGlobal, id);
}

void CompositorGlobal::CompositorGlobalPrivate::create_region (wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);
    CompositorGlobal *lCompositorGlobal = (CompositorGlobal*)wl_resource_get_user_data(resource);
    new RegionResource(lCompositorGlobal, id);
}

void CompositorGlobal::CompositorGlobalPrivate::resource_destroy(wl_resource *resource)
{
    CompositorGlobal *lCompositorGlobal = (CompositorGlobal*)wl_resource_get_user_data(resource);
    delete lCompositorGlobal;
}
