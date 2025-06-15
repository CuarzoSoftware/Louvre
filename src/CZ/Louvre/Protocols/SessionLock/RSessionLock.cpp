#include <CZ/Louvre/Protocols/SessionLock/ext-session-lock-v1.h>
#include <CZ/Louvre/Protocols/SessionLock/GSessionLockManager.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLockSurface.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLock.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
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
    else
    {
        m_timer.setCallback([this](auto)
        {
            makeLockRequest();
        });

        m_timer.start(2000);
    }
}

void RSessionLock::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RSessionLock*>(wl_resource_get_user_data(resource)) };

    if (res.reply() == RSessionLock::Locked)
    {
        res.postError(EXT_SESSION_LOCK_V1_ERROR_INVALID_DESTROY, "Attempted to destroy session lock while locked.");
        return;
    }

    wl_resource_destroy(resource);
}

void RSessionLock::get_lock_surface(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *output)
{
    auto &res { *static_cast<RSessionLock*>(wl_resource_get_user_data(resource)) };
    auto &surfaceRes { *static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };
    auto &outputRes { *static_cast<Wayland::GOutput*>(wl_resource_get_user_data(output)) };

    if (surfaceRes.surface()->role())
    {
        res.postError(EXT_SESSION_LOCK_V1_ERROR_ROLE, "Given wl_surface already has a role.");
        return;
    }

    if (surfaceRes.surface()->imp()->hasBufferOrPendingBuffer())
    {
        res.postError(EXT_SESSION_LOCK_V1_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface has a buffer attached or committed.");
        return;
    }

    for (auto *role : res.m_roles)
    {
        if (role->exclusiveOutput() && outputRes.output() && role->exclusiveOutput() == outputRes.output())
        {
            res.postError(EXT_SESSION_LOCK_V1_ERROR_DUPLICATE_OUTPUT, "Given output already has a lock surface.");
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
        res.postError(EXT_SESSION_LOCK_V1_ERROR_INVALID_UNLOCK, "Unlock requested but locked event was never sent.");
        return;
    }

    if (compositor()->sessionLockManager()->m_sessionLockRes == &res)
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

bool RSessionLock::makeLockRequest()
{
    m_timer.cancel();

    // Ask the user if the session should be locked
    if (sessionLockManager()->lockRequest(client()))
    {
        sessionLockManager()->m_state = LSessionLockManager::Locked;
        sessionLockManager()->m_sessionLockRes.reset(this);

        m_reply = RSessionLock::Locked;

        for (LSessionLockRole *role : roles())
        {
            role->exclusiveOutput()->imp()->sessionLockRole.reset(role);
            role->exclusiveOutput()->repaint();
            role->surface()->sendOutputEnterEvent(role->exclusiveOutput());
            role->surface()->imp()->setMapped(true);
            role->surface()->requestNextFrame(false);
        }

        sessionLockManager()->stateChanged();

        // The locked event is sent later after all outputs have been repainted
        for (LOutput *output : compositor()->outputs())
            m_pendingRepaint.emplace_back(output);

        return true;
    }

    finished();
    return false;
}
