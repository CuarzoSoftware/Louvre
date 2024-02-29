#include <protocols/GammaControl/private/GGammaControlManagerPrivate.h>
#include <protocols/GammaControl/RGammaControl.h>
#include <LCompositor.h>

struct zwlr_gamma_control_manager_v1_interface gamma_control_manager_implementation
{
    .get_gamma_control = &GGammaControlManager::GGammaControlManagerPrivate::get_gamma_control,
    .destroy = &GGammaControlManager::GGammaControlManagerPrivate::destroy
};

void GGammaControlManager::GGammaControlManagerPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);

    LClient *lClient { compositor()->getClientFromNativeResource(client) };
    new GGammaControlManager(lClient,
                    &zwlr_gamma_control_manager_v1_interface,
                    version,
                    id,
                    &gamma_control_manager_implementation);
}

void GGammaControlManager::GGammaControlManagerPrivate::get_gamma_control(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *output)
{
    L_UNUSED(client);

    Wayland::GOutput *gOutput { (Wayland::GOutput*)wl_resource_get_user_data(output) };

    new RGammaControl(gOutput, wl_resource_get_version(resource), id);
}

void GGammaControlManager::GGammaControlManagerPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client)
    wl_resource_destroy(resource);
}
