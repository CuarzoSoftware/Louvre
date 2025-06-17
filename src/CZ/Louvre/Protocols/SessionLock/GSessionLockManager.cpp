#include <CZ/Louvre/Protocols/SessionLock/ext-session-lock-v1.h>
#include <CZ/Louvre/Protocols/SessionLock/GSessionLockManager.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLock.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::SessionLock;

static const struct ext_session_lock_manager_v1_interface imp
{
    .destroy = &GSessionLockManager::destroy,
    .lock = &GSessionLockManager::lock
};

void GSessionLockManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GSessionLockManager(client, version, id);
}

Int32 GSessionLockManager::maxVersion() noexcept
{
    return LOUVRE_SESSION_LOCK_MANAGER_VERSION;
}

const wl_interface *GSessionLockManager::interface() noexcept
{
    return &ext_session_lock_manager_v1_interface;
}

GSessionLockManager::GSessionLockManager(wl_client *client, Int32 version, UInt32 id) noexcept :
    LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->sessionLockManagerGlobals.emplace_back(this);
}

GSessionLockManager::~GSessionLockManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->sessionLockManagerGlobals, this);
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
