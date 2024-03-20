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

    if (imp()->pendingPlaceAbove.get())
    {
        compositor()->imp()->insertSurfaceAfter(imp()->pendingPlaceAbove.get(), surface());
        surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
        surface()->imp()->parentLink = surface()->parent()->imp()->children.insert(
            std::next(imp()->pendingPlaceAbove.get()->imp()->parentLink),
            surface());
        placedAbove(imp()->pendingPlaceAbove.get());
        imp()->pendingPlaceAbove.reset();
    }

    if (imp()->pendingPlaceBelow.get())
    {
        compositor()->imp()->insertSurfaceBefore(imp()->pendingPlaceBelow.get(), surface());
        surface()->parent()->imp()->children.erase(surface()->imp()->parentLink);
        surface()->imp()->parentLink = surface()->parent()->imp()->children.insert(
            imp()->pendingPlaceBelow.get()->imp()->parentLink,
            surface());
        placedBelow(imp()->pendingPlaceBelow.get());
        imp()->pendingPlaceBelow.reset();
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
