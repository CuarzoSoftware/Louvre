#include <CZ/Louvre/Protocols/TearingControl/tearing-control-v1.h>
#include <CZ/Louvre/Protocols/TearingControl/GTearingControlManager.h>
#include <CZ/Louvre/Protocols/TearingControl/RTearingControl.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::TearingControl;

static const struct wp_tearing_control_manager_v1_interface imp
{
    .destroy = &GTearingControlManager::destroy,
    .get_tearing_control = &GTearingControlManager::get_tearing_control,
};

void GTearingControlManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GTearingControlManager(client, version, id);
}

Int32 GTearingControlManager::maxVersion() noexcept
{
    return LOUVRE_TEARING_CONTROL_MANAGER_VERSION;
}

const wl_interface *GTearingControlManager::interface() noexcept
{
    return &wp_tearing_control_manager_v1_interface;
}

GTearingControlManager::GTearingControlManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        client,
        interface(),
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

void GTearingControlManager::get_tearing_control(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    Wayland::RSurface *surfaceRes { static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->tearingControlRes())
    {
        surfaceRes->postError(
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
