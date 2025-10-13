#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RSubsurface.h>
#include <CZ/Louvre/Private/LSubsurfaceRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ;

LSubsurfaceRole::LSubsurfaceRole(const void *params) noexcept :
    LBaseSurfaceRole(
        FactoryObjectType,
        static_cast<const LSubsurfaceRole::Params*>(params)->subsurface,
        static_cast<const LSubsurfaceRole::Params*>(params)->surface,
        LSurface::Role::Subsurface)
{}

LSubsurfaceRole::~LSubsurfaceRole() noexcept
{
    validateDestructor();
    notifyDestruction();
}

void LSubsurfaceRole::handleParentCommit() noexcept
{
    if (m_pendingLocalPos != m_currentLocalPos)
    {
        m_currentLocalPos = m_pendingLocalPos;
        localPosChanged();
    }

    if (m_isSynced)
        m_lock.reset(); // This will commit the cached state
}

void LSubsurfaceRole::updateMapping() noexcept
{
    surface()->imp()->setMapped(surface()->parent() && surface()->parent()->mapped() && surface()->bufferResource());
}

void LSubsurfaceRole::cacheCommit() noexcept
{
    if (m_isSynced && !m_lock)
        m_lock = surface()->imp()->lock();
}

void LSubsurfaceRole::applyCommit() noexcept
{
    updateMapping();
}

void LSubsurfaceRole::handleParentMappingChange() noexcept
{
    updateMapping();
}
