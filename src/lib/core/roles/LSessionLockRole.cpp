#include <protocols/SessionLock/ext-session-lock-v1.h>
#include <protocols/SessionLock/RSessionLockSurface.h>
#include <protocols/SessionLock/RSessionLock.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>
#include <LCompositor.h>
#include <LTime.h>

using namespace Louvre::Protocols::SessionLock;
using namespace Louvre;

LSessionLockRole::LSessionLockRole(const void *params) noexcept :
    LBaseSurfaceRole(
        FactoryObjectType,
        static_cast<const Params*>(params)->resource,
        static_cast<const Params*>(params)->surface,
        LSurface::SessionLock
    ),
    m_output(static_cast<const Params*>(params)->output)
{}

void LSessionLockRole::handleSurfaceCommit(CommitOrigin /*origin*/)
{
    auto &sessionLockSurfaceRes { *static_cast<RSessionLockSurface*>(resource()) };

    if (!surface()->hasBuffer() || !sessionLockSurfaceRes.sessionLockRes())
    {
        wl_resource_post_error(sessionLockSurfaceRes.resource(),
                               EXT_SESSION_LOCK_SURFACE_V1_ERROR_NULL_BUFFER,
                               "Surface committed with a null buffer.");
        return;
    }

    auto &sessionLockRes { *sessionLockSurfaceRes.sessionLockRes() };

    if (m_currentSize.w() == -1)
    {
        wl_resource_post_error(sessionLockSurfaceRes.resource(),
                               EXT_SESSION_LOCK_SURFACE_V1_ERROR_COMMIT_BEFORE_FIRST_ACK,
                               "Surface committed before first ack_configure request.");
        return;
    }

    if (m_currentSize != surface()->size())
    {
        wl_resource_post_error(sessionLockSurfaceRes.resource(),
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

void LSessionLockRole::configure(const LSize &size) noexcept
{
    m_pendingSize = size;

    if (m_pendingSize.w() < 0)
        m_pendingSize.setW(0);

    if (m_pendingSize.h() < 0)
        m_pendingSize.setH(0);

    if (!m_hasPendingConf)
    {
        m_hasPendingConf = true;
        m_pendingSerial = LTime::nextSerial();
    }
}

void LSessionLockRole::sendPendingConfiguration() noexcept
{
    if (!m_hasPendingConf)
        return;

    m_hasPendingConf = false;
    m_sentConfs.emplace(m_pendingSize, m_pendingSerial);
    static_cast<Protocols::SessionLock::RSessionLockSurface*>(resource())->configure(
        m_pendingSerial,
        m_pendingSize.w(),
        m_pendingSize.h());
}

