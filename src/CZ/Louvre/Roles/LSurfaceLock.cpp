#include <CZ/Louvre/Roles/LSurfaceLock.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Events/LSurfaceUnlockEvent.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/CZCore.h>

using namespace CZ;

LSurfaceLock::~LSurfaceLock() noexcept
{
    if (!m_surface)
        return;

    LLog(CZTrace, CZLN, "Surface {}: Commit {} unlocked", m_surface->surfaceResource()->id(), m_commitId);

    if (!m_userCreated)
    {
        m_surface->imp()->unlockCommit(m_commitId);
        compositor()->imp()->unlockPoll();
    }
    else
        CZCore::Get()->postEvent(
            std::make_shared<LSurfaceUnlockEvent>(m_surface, m_commitId),
            *compositor());
}

LSurfaceLock::LSurfaceLock(LSurface *surface, bool userCreated) noexcept :
    m_surface(surface), m_userCreated(userCreated)
{
    m_commitId = m_surface->imp()->pending.commitId;
    LLog(CZTrace, CZLN, "Surface {}: Commit {} locked", m_surface->surfaceResource()->id(), m_commitId);
    m_surface->imp()->pending.lockCount++;
}
