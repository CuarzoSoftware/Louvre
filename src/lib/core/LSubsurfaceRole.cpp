#include <protocols/Wayland/RSurface.h>
#include <private/LBaseSurfaceRolePrivate.h>
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
        ((LSubsurfaceRole::Params*)params)->subsurface,
        ((LSubsurfaceRole::Params*)params)->surface,
        LSurface::Role::Subsurface
    ),
    LPRIVATE_INIT_UNIQUE(LSubsurfaceRole)
{}

LSubsurfaceRole::~LSubsurfaceRole()
{
    if (surface())
        surface()->imp()->setMapped(false);
}

bool LSubsurfaceRole::isSynced() const
{
    return imp()->isSynced;
}

const LPoint &LSubsurfaceRole::localPos() const
{
    return imp()->currentLocalPos;
}

bool LSubsurfaceRole::acceptCommitRequest(Wayland::RSurface::CommitOrigin origin)
{
    if (isSynced())
    {
        imp()->hasCache = true;
        return origin == Wayland::RSurface::Parent;
    }
    else
        return origin == Wayland::RSurface::Itself;
}

static void checkMapping(LSurface *surface)
{
    surface->imp()->setMapped(surface->parent() &&
                                surface->parent()->mapped() &&
                                surface->buffer());
}

void LSubsurfaceRole::handleSurfaceCommit(Wayland::RSurface::CommitOrigin origin)
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
        surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
        surface()->imp()->parentLink = surface()->parent()->imp()->children.insert(
            std::next(imp()->pendingPlaceAbove->imp()->parentLink),
            surface());
        placedAbove(imp()->pendingPlaceAbove);
        imp()->pendingPlaceAbove = nullptr;
        wl_list_remove(&imp()->pendingPlaceAboveDestroyListener.link);
    }

    if (imp()->pendingPlaceBelow)
    {
        compositor()->imp()->insertSurfaceBefore(imp()->pendingPlaceBelow, surface());
        surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
        surface()->imp()->parentLink = surface()->parent()->imp()->children.insert(
            imp()->pendingPlaceBelow->imp()->parentLink,
            surface());
        placedBelow(imp()->pendingPlaceBelow);
        imp()->pendingPlaceBelow = nullptr;
        wl_list_remove(&imp()->pendingPlaceBelowDestroyListener.link);
    }

    if (isSynced() && imp()->hasCache)
        Wayland::RSurface::apply_commit(surface(), Wayland::RSurface::Parent);
}

void LSubsurfaceRole::handleParentChange()
{
    checkMapping(surface());
}

void LSubsurfaceRole::handleParentMappingChange()
{
    checkMapping(surface());
}
