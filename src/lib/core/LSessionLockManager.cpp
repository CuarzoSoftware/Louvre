#include <protocols/SessionLock/RSessionLock.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>
#include <cassert>

using namespace Louvre;

LSessionLockManager::LSessionLockManager(const void *params)
{
    assert(params != nullptr && "Invalid parameter passed to LSessionLockManager constructor. LSessionLockManager can only be created from LCompositor::createSessionLockManagerRequest().");
    LSessionLockManager**ptr { (LSessionLockManager**) params };
    assert(*ptr == nullptr && *ptr == compositor()->sessionLockManager() && "Only a single LSessionLockManager instance can exist.");
    *ptr = this;

    m_sessionLockRes.setOnDestroyCallback([this](auto)
    {
        if (m_state == Locked)
        {
            m_sessionLockRes.reset();
            stateChanged();
        }
    });
}

LClient *LSessionLockManager::client() const noexcept
{
    return m_sessionLockRes == nullptr ? nullptr : m_sessionLockRes->client();
}

const std::vector<LSessionLockRole *> &LSessionLockManager::roles() const noexcept
{
    return m_sessionLockRes == nullptr ? m_dummy : m_sessionLockRes->roles();
}

void LSessionLockManager::forceUnlock()
{
    if (state() == Unlocked)
        return;

    if (m_sessionLockRes)
    {
        for (LSessionLockRole *role : roles())
            if (role->surface())
                role->surface()->imp()->setMapped(false);

        for (LOutput *output : seat()->outputs())
            output->imp()->sessionLockRole.reset();

        m_sessionLockRes->finished();
        m_sessionLockRes.reset();
    }

    m_state = Unlocked;
    stateChanged();
}

