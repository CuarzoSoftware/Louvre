#include <CZ/Louvre/Protocols/IdleInhibit/idle-inhibit-unstable-v1.h>
#include <CZ/Louvre/Protocols/IdleInhibit/GIdleInhibitManager.h>
#include <CZ/Louvre/Protocols/IdleInhibit/RIdleInhibitor.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::IdleInhibit;
using namespace CZ;

static const struct zwp_idle_inhibit_manager_v1_interface imp
{
    .destroy = &GIdleInhibitManager::destroy,
    .create_inhibitor = &GIdleInhibitManager::create_inhibitor
};

LGLOBAL_INTERFACE_IMP(GIdleInhibitManager, LOUVRE_IDLE_INHIBIT_MANAGER_VERSION, zwp_idle_inhibit_manager_v1_interface)

bool GIdleInhibitManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.IdleInhibitManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.IdleInhibitManager;
    return true;
}

GIdleInhibitManager::GIdleInhibitManager(
    wl_client *client,
    Int32 version,
    UInt32 id
    )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->idleInhibitManagerGlobals.emplace_back(this);
}

GIdleInhibitManager::~GIdleInhibitManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->idleInhibitManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GIdleInhibitManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GIdleInhibitManager::create_inhibitor(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    new RIdleInhibitor(static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface))->surface(),
                      wl_resource_get_version(resource),
                      id);
}

