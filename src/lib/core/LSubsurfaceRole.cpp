#include "LNamespaces.h"
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSubsurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <protocols/Wayland/private/SurfaceResourcePrivate.h>

using namespace Louvre;

LSubsurfaceRole::LSubsurfaceRole(Params *params) : LBaseSurfaceRole(params->subsurface,params->surface,LSurface::Role::Subsurface)
{
    m_imp = new LSubsurfaceRolePrivate();
}

LSubsurfaceRole::~LSubsurfaceRole()
{
    if(surface())
        surface()->imp()->setMapped(false);

    delete m_imp;
}

bool LSubsurfaceRole::isSynced() const
{
    return imp()->isSynced || (surface()->parent() && surface()->parent()->subsurface() && surface()->parent()->subsurface()->isSynced());
}

const LPoint &LSubsurfaceRole::localPosS() const
{
    return imp()->currentLocalPosS;
}

const LPoint &LSubsurfaceRole::localPosC() const
{
    return imp()->currentLocalPosC;
}

LSubsurfaceRole::LSubsurfaceRolePrivate *LSubsurfaceRole::imp() const
{
    return m_imp;
}

bool LSubsurfaceRole::acceptCommitRequest(Protocols::Wayland::SurfaceResource::CommitOrigin origin)
{
    if(isSynced())
        return origin == Protocols::Wayland::SurfaceResource::Parent;
    else
        return origin == Protocols::Wayland::SurfaceResource::Itself;
}

void insertSurfaceAfter(list<LSurface*>&surfaces,LSurface *prevSurface, LSurface *surfaceToInsert)
{
    for(list<LSurface*>::iterator it = surfaces.begin(); it != surfaces.end(); it++)
    {
        if((*it) == prevSurface)
        {
            surfaces.remove(surfaceToInsert);

            if(it++ == surfaces.end())
                surfaces.push_back(surfaceToInsert);
            else
                surfaces.insert((it++),surfaceToInsert);

        }
    }
}

void insertSurfaceBefore(list<LSurface*>&surfaces,LSurface *nextSurface, LSurface *surfaceToInsert)
{
    for(list<LSurface*>::iterator it = surfaces.begin(); it != surfaces.end(); ++it)
    {
        if((*it) == nextSurface)
        {
            surfaces.remove(surfaceToInsert);
            surfaces.insert(it,surfaceToInsert);
            return;
        }
    }
}

void LSubsurfaceRole::handleSurfaceCommit()
{
    if(!surface()->mapped() && surface()->imp()->current.buffer && surface()->parent() && ((surface()->parent()->mapped() && isSynced()) || !isSynced() ))
    {
        surface()->imp()->setMapped(true);
    }

    else if(surface()->mapped() && (!surface()->imp()->current.buffer || !surface()->parent() || (!surface()->parent()->mapped() && isSynced())))
    {
        surface()->imp()->setMapped(false);
    }
}

void LSubsurfaceRole::handleParentCommit()
{
    if(imp()->hasPendingLocalPos)
    {
        imp()->hasPendingLocalPos = false;
        imp()->currentLocalPosS = imp()->pendingLocalPosS;
        imp()->currentLocalPosC = imp()->pendingLocalPosS * compositor()->globalScale();
        localPosChanged();
    }

    if(imp()->pendingPlaceAbove)
    {
        compositor()->imp()->insertSurfaceAfter(imp()->pendingPlaceAbove, surface());
        insertSurfaceAfter(surface()->imp()->children, imp()->pendingPlaceAbove, surface());
        placedAbove(imp()->pendingPlaceAbove);
        imp()->pendingPlaceAbove = nullptr;
    }

    if(imp()->pendingPlaceBelow)
    {
        compositor()->imp()->insertSurfaceBefore(imp()->pendingPlaceBelow, surface());
        insertSurfaceBefore(surface()->imp()->children, imp()->pendingPlaceBelow, surface());
        placedBelow(imp()->pendingPlaceBelow);
        imp()->pendingPlaceBelow = nullptr;
    }

    Protocols::Wayland::SurfaceResource::SurfaceResourcePrivate::apply_commit(surface(), Protocols::Wayland::SurfaceResource::Parent);
}

void LSubsurfaceRole::handleParentChange()
{
    surface()->imp()->setMapped(surface()->parent() && surface()->parent()->mapped() && surface()->imp()->pending.buffer);
}

void LSubsurfaceRole::handleParentMappingChange()
{
    surface()->imp()->setMapped(surface()->parent() && surface()->parent()->mapped() && surface()->imp()->pending.buffer);
}

void LSubsurfaceRole::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);

    // Local pos
    imp()->currentLocalPosC = imp()->pendingLocalPosS * newScale;
}


