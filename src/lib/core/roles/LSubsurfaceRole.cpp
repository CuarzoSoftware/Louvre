#include <protocols/Wayland/RSurface.h>
#include <private/LSubsurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>

using namespace Louvre;

LSubsurfaceRole::LSubsurfaceRole
(
    const void *params
)
    :LBaseSurfaceRole
    (
        static_cast<const LSubsurfaceRole::Params*>(params)->subsurface,
        static_cast<const LSubsurfaceRole::Params*>(params)->surface,
        LSurface::Role::Subsurface
    ),
    LPRIVATE_INIT_UNIQUE(LSubsurfaceRole)
{}

LSubsurfaceRole::~LSubsurfaceRole(){/* TODO: inline */}

bool LSubsurfaceRole::isSynced() const
{
    return imp()->isSynced;
}

const LPoint &LSubsurfaceRole::localPos() const
{
    return imp()->currentLocalPos;
}

bool LSubsurfaceRole::acceptCommitRequest(LBaseSurfaceRole::CommitOrigin origin)
{
    if (isSynced())
    {
        imp()->hasCache = true;
        return origin == LBaseSurfaceRole::CommitOrigin::Parent;
    }
    else
        return origin == LBaseSurfaceRole::CommitOrigin::Itself;
}

static void checkMapping(LSurface *surface)
{
    surface->imp()->setMapped(surface->parent() &&
                                surface->parent()->mapped() &&
                                surface->buffer());
}

void LSubsurfaceRole::handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin)
{
    L_UNUSED(origin);
    imp()->hasCache = false;
    checkMapping(surface());
}

void LSubsurfaceRole::handleParentCommit()
{
    if (imp()->hasPendingLocalPos)
    {
        imp()->hasPendingLocalPos = false;
        imp()->currentLocalPos = imp()->pendingLocalPos;
        localPosChanged();
    }

    if (imp()->pendingPlaceAbove)
    {
        compositor()->imp()->insertSurfaceAfter(imp()->pendingPlaceAbove, surface());

        if (imp()->pendingPlaceAbove == surface()->parent())
        {
            if (surface()->parent()->children().front() != surface())
            {
                surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
                surface()->parent()->imp()->children.push_front(surface());
                surface()->imp()->parentLink = surface()->parent()->imp()->children.begin();
                surface()->parent()->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
                placedAbove(imp()->pendingPlaceAbove);
            }
        }
        else if (*std::next(imp()->pendingPlaceAbove->imp()->parentLink) != surface())
        {
            surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
            surface()->imp()->parentLink = surface()->parent()->imp()->children.insert(
                std::next(imp()->pendingPlaceAbove->imp()->parentLink),
                surface());
            surface()->parent()->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
            placedAbove(imp()->pendingPlaceAbove);
        }

        imp()->pendingPlaceAbove.reset();
    }

    if (imp()->pendingPlaceBelow)
    {
        if (*std::prev(imp()->pendingPlaceAbove->imp()->parentLink) != surface())
        {
            compositor()->imp()->insertSurfaceBefore(imp()->pendingPlaceBelow, surface());
            surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
            surface()->imp()->parentLink = surface()->parent()->imp()->children.insert(
                imp()->pendingPlaceBelow->imp()->parentLink,
                surface());
            surface()->parent()->imp()->stateFlags.add(LSurface::LSurfacePrivate::ChildrenListChanged);
            placedBelow(imp()->pendingPlaceBelow);
        }

        imp()->pendingPlaceBelow.reset();
    }

    if (isSynced() && imp()->hasCache)
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
