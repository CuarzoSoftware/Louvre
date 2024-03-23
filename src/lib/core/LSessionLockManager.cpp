#include <protocols/SessionLock/RSessionLock.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>

using namespace Louvre;

LSessionLockManager::LSessionLockManager(const void *params)
{
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
    return m_sessionLockRes.get() == nullptr ? nullptr : m_sessionLockRes.get()->client();
}

const std::vector<LSessionLockRole *> &LSessionLockManager::roles() const noexcept
{
    return m_sessionLockRes.get() == nullptr ? m_dummy : m_sessionLockRes.get()->roles();
}

void LSessionLockManager::forceUnlock()
{
    if (state() == Unlocked)
        return;

    if (m_sessionLockRes.get())
    {
        for (LSessionLockRole *role : roles())
            if (role->surface())
                role->surface()->imp()->setMapped(false);

        for (LOutput *output : seat()->outputs())
            output->imp()->sessionLockRole.reset();

        m_sessionLockRes.get()->finished();
        m_sessionLockRes.reset();
    }

    m_state = Unlocked;
    stateChanged();
}

