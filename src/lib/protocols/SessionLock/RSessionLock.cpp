#include <protocols/SessionLock/ext-session-lock-v1.h>
#include <protocols/SessionLock/GSessionLockManager.h>
#include <protocols/SessionLock/RSessionLockSurface.h>
#include <protocols/SessionLock/RSessionLock.h>
#include <protocols/Wayland/GOutput.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>

using namespace Louvre::Protocols::SessionLock;

static const struct ext_session_lock_v1_interface imp
{
    .destroy = &RSessionLock::destroy,
    .get_lock_surface = &RSessionLock::get_lock_surface,
    .unlock_and_destroy = &RSessionLock::unlock_and_destroy
};

RSessionLock::RSessionLock(GSessionLockManager *sessionLockManagerRes,
    UInt32 id) noexcept :
    LResource
    (
        sessionLockManagerRes->client(),
        &ext_session_lock_v1_interface,
        sessionLockManagerRes->version(),
        id,
        &imp
    )
{
    /* Wait for the client to create surfaces for each output, then ask the user */

    if (compositor()->sessionLockManager()->state() == LSessionLockManager::Locked)
        finished();
}

void RSessionLock::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RSessionLock*>(wl_resource_get_user_data(resource)) };

    if (res.reply() == RSessionLock::Locked)
    {
        wl_resource_post_error(resource, EXT_SESSION_LOCK_V1_ERROR_INVALID_DESTROY, "Attempted to destroy session lock while locked.");
        return;
    }

    wl_resource_destroy(resource);
}

void RSessionLock::get_lock_surface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *output)
{
    auto &res { *static_cast<RSessionLock*>(wl_resource_get_user_data(resource)) };
    auto &surfaceRes { *static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };
    auto &outputRes { *static_cast<Wayland::GOutput*>(wl_resource_get_user_data(output)) };

    if (surfaceRes.surface()->imp()->hasRoleOrPendingRole())
    {
        wl_resource_post_error(resource, EXT_SESSION_LOCK_V1_ERROR_ROLE, "Given wl_surface already has a role.");
        return;
    }

    if (surfaceRes.surface()->imp()->hasBufferOrPendingBuffer())
    {
        wl_resource_post_error(resource, EXT_SESSION_LOCK_V1_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface has a buffer attached or committed.");
        return;
    }

    for (auto *role : res.m_roles)
    {
        if (role->output() && outputRes.output() && role->output() == outputRes.output())
        {
            wl_resource_post_error(resource, EXT_SESSION_LOCK_V1_ERROR_DUPLICATE_OUTPUT, "Given output already has a lock surface.");
            return;
        }
    }

    new RSessionLockSurface(&res, surfaceRes.surface(), outputRes.output(), id);
}

void RSessionLock::unlock_and_destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RSessionLock*>(wl_resource_get_user_data(resource)) };

    if (!res.m_lockedOnce)
    {
        wl_resource_post_error(resource, EXT_SESSION_LOCK_V1_ERROR_INVALID_UNLOCK, "Unlock requested but locked event was never sent.");
        return;
    }

    if (compositor()->sessionLockManager()->m_sessionLockRes.get() == &res)
    {

        for (LSessionLockRole *role : res.roles())
            if (role->surface())
                role->surface()->imp()->setMapped(false);

        for (LOutput *output : seat()->outputs())
            output->imp()->sessionLockRole.reset();

        compositor()->sessionLockManager()->m_sessionLockRes.reset();
        compositor()->sessionLockManager()->m_state = LSessionLockManager::Unlocked;
        compositor()->sessionLockManager()->stateChanged();
    }

    wl_resource_destroy(resource);
}

void RSessionLock::locked() noexcept
{
    m_lockedOnce = true;
    m_reply = Locked;
    ext_session_lock_v1_send_locked(resource());
}

void RSessionLock::finished() noexcept
{
    m_reply = Finished;
    ext_session_lock_v1_send_finished(resource());
}
