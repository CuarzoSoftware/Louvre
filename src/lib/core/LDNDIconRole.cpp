#include <private/LDNDIconRolePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>

#include <LSurface.h>
#include <LCompositor.h>

using namespace Louvre;

LDNDIconRole::LDNDIconRole(Params *params) : LBaseSurfaceRole(params->surface->surfaceResource(), params->surface, LSurface::Role::DNDIcon)
{
    m_imp = new LDNDIconRolePrivate();

    surface()->imp()->receiveInput = false;
}

LDNDIconRole::~LDNDIconRole()
{
    if (surface())
        surface()->imp()->setMapped(false);

    delete m_imp;
}

const LPoint &LDNDIconRole::hotspotS() const
{
    return imp()->currentHotspotS;
}

const LPoint &LDNDIconRole::hotspotC() const
{
    return imp()->currentHotspotC;
}

const LPoint &LDNDIconRole::hotspotB() const
{
    return imp()->currentHotspotB;
}

void LDNDIconRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    imp()->pendingHotspotOffsetS = LPoint(x,y);
}

void LDNDIconRole::handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin)
{
    imp()->currentHotspotS -= imp()->pendingHotspotOffsetS;
    imp()->pendingHotspotOffsetS = LPoint();
    imp()->currentHotspotC = imp()->currentHotspotS * compositor()->globalScale();
    imp()->currentHotspotB = imp()->currentHotspotS * surface()->bufferScale();
    hotspotChanged();
}

void LDNDIconRole::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);

    // Hotspot
    imp()->currentHotspotC = imp()->currentHotspotS * newScale;
}

