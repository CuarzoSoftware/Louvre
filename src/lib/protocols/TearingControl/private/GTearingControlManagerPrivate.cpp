#include <protocols/TearingControl/private/GTearingControlManagerPrivate.h>
#include <protocols/TearingControl/RTearingControl.h>
#include <protocols/Wayland/RSurface.h>
#include <LCompositor.h>

struct wp_tearing_control_manager_v1_interface tearing_control_manager_implementation
{
    .destroy = &GTearingControlManager::GTearingControlManagerPrivate::destroy,
    .get_tearing_control = &GTearingControlManager::GTearingControlManagerPrivate::get_tearing_control,
};

void GTearingControlManager::GTearingControlManagerPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);

    LClient *lClient = compositor()->getClientFromNativeResource(client);
    new GTearingControlManager(lClient,
                             &wp_tearing_control_manager_v1_interface,
                             version,
                             id,
                             &tearing_control_manager_implementation);
}

void GTearingControlManager::GTearingControlManagerPrivate::get_tearing_control(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface)
{
    Wayland::RSurface *surfaceRes { static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->tearingControlRes())
    {
        wl_resource_post_error(resource,
                               WP_TEARING_CONTROL_MANAGER_V1_ERROR_TEARING_CONTROL_EXISTS,
                               "The surface already has a tearing object associated");
        return;
    }

    new RTearingControl(surfaceRes, wl_resource_get_version(resource), id);
}

void GTearingControlManager::GTearingControlManagerPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client)
    wl_resource_destroy(resource);
}
