#include <CZ/Louvre/Protocols/SessionLock/ext-session-lock-v1.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLockSurface.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLock.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Core/CZTime.h>

using namespace CZ::Protocols::SessionLock;
using namespace CZ;

LSessionLockRole::LSessionLockRole(const void *params) noexcept :
    LBaseSurfaceRole(
        FactoryObjectType,
        static_cast<const Params*>(params)->resource,
        static_cast<const Params*>(params)->surface,
        LSurface::SessionLock
    ),
    m_output(static_cast<const Params*>(params)->output)
{}

LSessionLockRole::~LSessionLockRole() noexcept
{
    validateDestructor();
    notifyDestruction();
}

void LSessionLockRole::configure(const SkISize &size) noexcept
{
    m_pendingConf.size = size;

    if (m_pendingConf.size.width() < 0)
        m_pendingConf.size.fWidth = 0;

    if (m_pendingConf.size.height() < 0)
        m_pendingConf.size.fHeight = 0;

    if (!m_hasPendingConf)
    {
        m_hasPendingConf = true;
        m_pendingConf.serial = CZTime::NextSerial();
    }
}

void LSessionLockRole::sendPendingConfiguration() noexcept
{
    if (!m_hasPendingConf)
        return;

    m_hasPendingConf = false;
    m_sentConfs.emplace(m_pendingConf);
    static_cast<Protocols::SessionLock::RSessionLockSurface*>(resource())->configure(
        m_pendingConf.serial,
        m_pendingConf.size.width(),
        m_pendingConf.size.height());
}

void LSessionLockRole::cacheCommit() noexcept
{
    if (surface()->isLocked())
        m_cache.emplace_back(m_pending);
}

void LSessionLockRole::applyCommit() noexcept
{
    auto &sessionLockSurfaceRes { *static_cast<RSessionLockSurface*>(resource()) };

    if (m_cache.empty())
        m_current = m_pending;
    else
    {
        m_current = m_cache.front();
        m_cache.pop_front();
    }

    if (!surface()->bufferResource() || !sessionLockSurfaceRes.sessionLockRes())
    {
        sessionLockSurfaceRes.postError(
            EXT_SESSION_LOCK_SURFACE_V1_ERROR_NULL_BUFFER,
            "Surface committed with a null buffer.");
        return;
    }

    auto &sessionLockRes { *sessionLockSurfaceRes.sessionLockRes() };

    if (m_current.size.width() == -1)
    {
        sessionLockSurfaceRes.postError(
            EXT_SESSION_LOCK_SURFACE_V1_ERROR_COMMIT_BEFORE_FIRST_ACK,
            "Surface committed before first ack_configure request.");
        return;
    }

    if (m_current.size != surface()->size())
    {
        sessionLockSurfaceRes.postError(
            EXT_SESSION_LOCK_SURFACE_V1_ERROR_DIMENSIONS_MISMATCH,
            "Failed to match ack'd width/height.");
        return;
    }

    m_isComplete = true;

    switch (sessionLockRes.reply())
    {
    case RSessionLock::Undefined:
    {
        // The session is already locked
        if (sessionLockManager()->state() == LSessionLockManager::Locked)
        {
            sessionLockRes.finished();
            return;
        }

        // Check if there is a role for each output
        for (LOutput *output : compositor()->outputs())
        {
            bool roleForOutput { false };

            for (auto *role : sessionLockRes.roles())
            {
                if (!role->m_isComplete)
                    return;

                if (role->exclusiveOutput() == output)
                {
                    roleForOutput = true;
                    break;
                }
            }

            if (!roleForOutput)
                return;
        }

        sessionLockRes.makeLockRequest();

    } break;
    case RSessionLock::Locked:
    {
        if (exclusiveOutput() && exclusiveOutput()->state() != LOutput::Uninitialized)
        {
            exclusiveOutput()->imp()->sessionLockRole.reset(this);
            surface()->sendOutputEnterEvent(exclusiveOutput());
            surface()->imp()->setMapped(true);
            exclusiveOutput()->repaint();
        }
    } break;
    case RSessionLock::Finished:
        break;
    };
}

