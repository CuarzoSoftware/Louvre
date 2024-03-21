#include <protocols/TearingControl/GTearingControlManager.h>
#include <protocols/TearingControl/RTearingControl.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::TearingControl;

static const struct wp_tearing_control_manager_v1_interface imp
{
    .destroy = &GTearingControlManager::destroy,
    .get_tearing_control = &GTearingControlManager::get_tearing_control,
};

GTearingControlManager::GTearingControlManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        client,
        &wp_tearing_control_manager_v1_interface,
        version,
        id,
        &imp
    )
{
    this->client()->imp()->tearingControlManagerGlobals.push_back(this);
}

GTearingControlManager::~GTearingControlManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->tearingControlManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GTearingControlManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GTearingControlManager(client, version, id);
}

void GTearingControlManager::get_tearing_control(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
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

void GTearingControlManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
