#include <CZ/Louvre/Protocols/SessionLock/ext-session-lock-v1.h>
#include <CZ/Louvre/Protocols/SessionLock/GSessionLockManager.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLock.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::SessionLock;

static const struct ext_session_lock_manager_v1_interface imp
{
    .destroy = &GSessionLockManager::destroy,
    .lock = &GSessionLockManager::lock
};

LGLOBAL_INTERFACE_IMP(GSessionLockManager, LOUVRE_SESSION_LOCK_MANAGER_VERSION, ext_session_lock_manager_v1_interface)

bool GSessionLockManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.SessionLockManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.SessionLockManager;
    return true;
}

GSessionLockManager::GSessionLockManager(wl_client *client, Int32 version, UInt32 id) :
    LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->sessionLockManagerGlobals.emplace_back(this);
}

GSessionLockManager::~GSessionLockManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->sessionLockManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GSessionLockManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GSessionLockManager::lock(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    new RSessionLock(static_cast<GSessionLockManager*>(wl_resource_get_user_data(resource)), id);
}
