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

LSubsurfaceRole::~LSubsurfaceRole()
{
    validateDestructor();
    notifyDestruction();
}

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

    if (m_pendingPlace)
    {
        LSurface *parent { surface()->parent() };

        if (m_pendingPlaceAbove)
        {
            if (m_pendingPlace == parent)
            {
                LSurface *lastBelowParent { nullptr }; // The surface below the parent, closest to the parent

                for (auto it = parent->imp()->children.rbegin(); it != parent->imp()->children.rend(); it++)
                {
                    if (!(*it)->imp()->stateFlags.check(LSurface::LSurfacePrivate::AboveParent))
                    {
                        lastBelowParent = *it;
                        break;
                    }
                }

                if (!lastBelowParent)
                {
                    if (parent->children().front() != surface())
                    {
                        compositor()->imp()->insertSurfaceAfter(m_pendingPlace, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
                        parent->imp()->children.erase(surface()->imp()->parentLink);
                        parent->imp()->children.push_front(surface());
                        surface()->imp()->parentLink = parent->imp()->children.begin();
                        parent->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                        surface()->imp()->stateFlags.add(LSurface::LSurfacePrivate::AboveParent);
                        placedAbove(m_pendingPlace);
                    }
                }
                else
                {
                    if (lastBelowParent == surface())
                    {
                        compositor()->imp()->insertSurfaceAfter(m_pendingPlace, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
                        parent->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                        surface()->imp()->stateFlags.add(LSurface::LSurfacePrivate::AboveParent);
                        placedAbove(m_pendingPlace);
                    }
                    else if (lastBelowParent == parent->imp()->children.back())
                    {
                        compositor()->imp()->insertSurfaceAfter(m_pendingPlace, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
                        parent->imp()->children.erase(surface()->imp()->parentLink);
                        parent->imp()->children.push_back(surface());
                        surface()->imp()->parentLink = std::prev(parent->imp()->children.end());
                        parent->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                        surface()->imp()->stateFlags.add(LSurface::LSurfacePrivate::AboveParent);
                        placedAbove(m_pendingPlace);
                    }
                    else
                    {
                        compositor()->imp()->insertSurfaceAfter(m_pendingPlace, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
                        parent->imp()->children.erase(surface()->imp()->parentLink);
                        surface()->imp()->parentLink = parent->imp()->children.insert(std::next(lastBelowParent->imp()->pendingParentLink), surface());
                        parent->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                        surface()->imp()->stateFlags.add(LSurface::LSurfacePrivate::AboveParent);
                        placedAbove(m_pendingPlace);
                    }
                }
            }
            else
            {
                auto next { std::next(m_pendingPlace->imp()->parentLink) };

                if (next == parent->imp()->children.end())
                {
                    compositor()->imp()->insertSurfaceAfter(m_pendingPlace, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
                    parent->imp()->children.erase(surface()->imp()->parentLink);
                    parent->imp()->children.emplace_back(surface());
                    surface()->imp()->parentLink = std::prev(parent->imp()->children.end());
                    parent->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                    surface()->imp()->stateFlags.setFlag(LSurface::LSurfacePrivate::AboveParent, m_pendingPlace->imp()->stateFlags.check(LSurface::LSurfacePrivate::AboveParent));
                    placedAbove(m_pendingPlace);
                }
                else if (*next != surface()) // Otherwise its already above the sibling
                {
                    compositor()->imp()->insertSurfaceAfter(m_pendingPlace, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
                    parent->imp()->children.erase(surface()->imp()->parentLink);
                    surface()->imp()->parentLink = parent->imp()->children.insert(
                        next,
                        surface());
                    parent->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                    surface()->imp()->stateFlags.setFlag(LSurface::LSurfacePrivate::AboveParent, m_pendingPlace->imp()->stateFlags.check(LSurface::LSurfacePrivate::AboveParent));
                    placedAbove(m_pendingPlace);
                }
            }

        }
        else // Place below
        {
            if (m_pendingPlace == parent)
            {
                LSurface *lastBelowParent { nullptr }; // The surface below the parent, closest to the parent

                for (auto it = parent->imp()->children.rbegin(); it != parent->imp()->children.rend(); it++)
                {
                    if (!(*it)->imp()->stateFlags.check(LSurface::LSurfacePrivate::AboveParent))
                    {
                        lastBelowParent = *it;
                        break;
                    }
                }

                if (lastBelowParent == nullptr)
                {
                    compositor()->imp()->insertSurfaceBefore(m_pendingPlace, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
                    parent->imp()->children.erase(surface()->imp()->parentLink);
                    parent->imp()->children.push_front(surface());
                    surface()->imp()->parentLink = parent->imp()->children.begin();
                    parent->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                    surface()->imp()->stateFlags.remove(LSurface::LSurfacePrivate::AboveParent);
                    placedBelow(m_pendingPlace);
                }
                else if (lastBelowParent != surface())
                {
                    compositor()->imp()->insertSurfaceBefore(m_pendingPlace, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
                    parent->imp()->children.erase(surface()->imp()->parentLink);

                    surface()->imp()->parentLink = parent->imp()->children.insert(
                        m_pendingPlace->imp()->parentLink,
                        surface());
                    parent->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                    surface()->imp()->stateFlags.remove(LSurface::LSurfacePrivate::AboveParent);
                    placedBelow(m_pendingPlace);
                }
            }
            else if (parent->imp()->children.front() == m_pendingPlace || *std::prev(m_pendingPlace->imp()->parentLink) != surface())
            {
                compositor()->imp()->insertSurfaceBefore(m_pendingPlace, surface(), OP::UpdateSurfaces | OP::UpdateLayers);
                parent->imp()->children.erase(surface()->imp()->parentLink);
                surface()->imp()->parentLink = parent->imp()->children.insert(
                    m_pendingPlace->imp()->parentLink,
                    surface());
                parent->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                surface()->imp()->stateFlags.setFlag(LSurface::LSurfacePrivate::AboveParent, m_pendingPlace->imp()->stateFlags.check(LSurface::LSurfacePrivate::AboveParent));
                placedBelow(m_pendingPlace);
            }
        }
    }

    m_pendingPlace.reset();

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
