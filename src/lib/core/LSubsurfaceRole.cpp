#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSubsurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>

using namespace Louvre;

LSubsurfaceRole::LSubsurfaceRole
(
    Params *params
)
    :LBaseSurfaceRole
    (
        params->subsurface,
        params->surface,
        LSurface::Role::Subsurface
    )
{
    m_imp = new LSubsurfaceRolePrivate();
}

LSubsurfaceRole::~LSubsurfaceRole()
{
    if (surface())
        surface()->imp()->setMapped(false);

    delete m_imp;
}

bool LSubsurfaceRole::isSynced() const
{
    return imp()->isSynced;
}

const LPoint &LSubsurfaceRole::localPosS() const
{
    return imp()->currentLocalPosS;
}

const LPoint &LSubsurfaceRole::localPosC() const
{
    return imp()->currentLocalPosC;
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
                                surface->imp()->pending.buffer);
}

void LSubsurfaceRole::handleSurfaceCommit(Wayland::RSurface::CommitOrigin origin)
{
    L_UNUSED(origin);
    imp()->hasCache = true;
    checkMapping(surface());
}

void LSubsurfaceRole::handleParentCommit()
{
    if (imp()->hasPendingLocalPos)
    {
        imp()->hasPendingLocalPos = false;
        imp()->currentLocalPosS = imp()->pendingLocalPosS;
        imp()->currentLocalPosC = imp()->pendingLocalPosS * compositor()->globalScale();
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
        Wayland::RSurface::RSurfacePrivate::apply_commit(surface(), Wayland::RSurface::Parent);
}

void LSubsurfaceRole::handleParentChange()
{
    checkMapping(surface());
}

void LSubsurfaceRole::handleParentMappingChange()
{
    checkMapping(surface());
}

void LSubsurfaceRole::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);

    // Local pos
    imp()->currentLocalPosC = imp()->pendingLocalPosS * newScale;
}
