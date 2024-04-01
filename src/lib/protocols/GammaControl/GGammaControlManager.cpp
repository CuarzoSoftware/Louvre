#include <protocols/GammaControl/wlr-gamma-control-unstable-v1.h>
#include <protocols/GammaControl/GGammaControlManager.h>
#include <protocols/GammaControl/RGammaControl.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::GammaControl;

static const struct zwlr_gamma_control_manager_v1_interface imp
{
    .get_gamma_control = &GGammaControlManager::get_gamma_control,
    .destroy = &GGammaControlManager::destroy
};

void GGammaControlManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GGammaControlManager(client, version, id);
}

Int32 GGammaControlManager::maxVersion() noexcept
{
    return LOUVRE_GAMMA_CONTROL_MANAGER_VERSION;
}

const wl_interface *GGammaControlManager::interface() noexcept
{
    return &zwlr_gamma_control_manager_v1_interface;
}

GGammaControlManager::GGammaControlManager(
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
    this->client()->imp()->gammaControlManagerGlobals.emplace_back(this);
}

GGammaControlManager::~GGammaControlManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->gammaControlManagerGlobals, this);
}

void GGammaControlManager::get_gamma_control(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *output) noexcept
{
    new RGammaControl(static_cast<Wayland::GOutput*>(wl_resource_get_user_data(output)),
                      wl_resource_get_version(resource),
                      id);
}

/******************** REQUESTS ********************/

void GGammaControlManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
