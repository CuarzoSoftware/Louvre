#include <protocols/Wayland/RSurface.h>
#include <private/LSubsurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>

using namespace Louvre;

LSubsurfaceRole::LSubsurfaceRole(const void *params) noexcept :
    LBaseSurfaceRole(
        FactoryObjectType,
        static_cast<const LSubsurfaceRole::Params*>(params)->subsurface,
        static_cast<const LSubsurfaceRole::Params*>(params)->surface,
        LSurface::Role::Subsurface)
{}

bool LSubsurfaceRole::acceptCommitRequest(LBaseSurfaceRole::CommitOrigin origin)
{
    if (isSynced())
    {
        m_hasCache = true;
        return origin == LBaseSurfaceRole::CommitOrigin::Parent;
    }
    else
        return origin == LBaseSurfaceRole::CommitOrigin::Itself;
}

static void checkMapping(LSurface *surface)
{
    surface->imp()->setMapped(surface->parent() &&
                                surface->parent()->mapped() &&
                                surface->hasBuffer());
}

void LSubsurfaceRole::handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin)
{
    L_UNUSED(origin);
    m_hasCache = false;
    checkMapping(surface());
}

void LSubsurfaceRole::handleParentCommit()
{
    using OP = LCompositor::LCompositorPrivate::InsertOptions;

    if (m_hasPendingLocalPos)
    {
        m_hasPendingLocalPos = false;
        m_currentLocalPos = m_pendingLocalPos;
        localPosChanged();
    }

    if (m_pendingPlaceAbove)
    {
        compositor()->imp()->insertSurfaceAfter(m_pendingPlaceAbove, surface(), OP::UpdateSurfaces | OP::UpdateLayers);

        if (m_pendingPlaceAbove == surface()->parent())
        {
            if (surface()->parent()->children().front() != surface())
            {
                surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
                surface()->parent()->imp()->children.push_front(surface());
                surface()->imp()->parentLink = surface()->parent()->imp()->children.begin();
                surface()->parent()->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                placedAbove(m_pendingPlaceAbove);
            }
        }
        else if (*std::next(m_pendingPlaceAbove->imp()->parentLink) != surface())
        {
            surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
            surface()->imp()->parentLink = surface()->parent()->imp()->children.insert(
                std::next(m_pendingPlaceAbove->imp()->parentLink),
                surface());
            surface()->parent()->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
            placedAbove(m_pendingPlaceAbove);
        }

        m_pendingPlaceAbove.reset();
    }

    if (m_pendingPlaceBelow)
    {
        if (*std::prev(m_pendingPlaceAbove->imp()->parentLink) != surface())
        {
            compositor()->imp()->insertSurfaceBefore(m_pendingPlaceBelow, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
            surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
            surface()->imp()->parentLink = surface()->parent()->imp()->children.insert(
                m_pendingPlaceBelow->imp()->parentLink,
                surface());
            surface()->parent()->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
            placedBelow(m_pendingPlaceBelow);
        }

        m_pendingPlaceBelow.reset();
    }

    if (isSynced() && m_hasCache)
        Wayland::RSurface::apply_commit(surface(), LBaseSurfaceRole::CommitOrigin::Parent);
}

void LSubsurfaceRole::handleParentChange()
{
    checkMapping(surface());
}

void LSubsurfaceRole::handleParentMappingChange()
{
    checkMapping(surface());
}
