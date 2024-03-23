#include <protocols/SessionLock/GSessionLockManager.h>
#include <protocols/SessionLock/RSessionLock.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::SessionLock;

static const struct ext_session_lock_manager_v1_interface imp
{
    .destroy = &GSessionLockManager::destroy,
    .lock = &GSessionLockManager::lock
};

GSessionLockManager::GSessionLockManager(wl_client *client, Int32 version, UInt32 id) noexcept :
    LResource
    (
        client,
        &ext_session_lock_manager_v1_interface,
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

void GSessionLockManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GSessionLockManager(client, version, id);
}

void GSessionLockManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GSessionLockManager::lock(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    new RSessionLock(static_cast<GSessionLockManager*>(wl_resource_get_user_data(resource)), id);
}
