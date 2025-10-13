#include <CZ/Louvre/Protocols/TearingControl/tearing-control-v1.h>
#include <CZ/Louvre/Protocols/TearingControl/GTearingControlManager.h>
#include <CZ/Louvre/Protocols/TearingControl/RTearingControl.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::TearingControl;

static const struct wp_tearing_control_manager_v1_interface imp
{
    .destroy = &GTearingControlManager::destroy,
    .get_tearing_control = &GTearingControlManager::get_tearing_control,
};

LGLOBAL_INTERFACE_IMP(GTearingControlManager, LOUVRE_TEARING_CONTROL_MANAGER_VERSION, wp_tearing_control_manager_v1_interface)

bool GTearingControlManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.TearingControlManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.TearingControlManager;
    return true;
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
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->tearingControlManagerGlobals.push_back(this);
}

GTearingControlManager::~GTearingControlManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->tearingControlManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GTearingControlManager::get_tearing_control(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    Wayland::RWlSurface *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

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
