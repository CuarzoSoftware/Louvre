#include <private/LDNDIconRolePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>

#include <LSurface.h>
#include <LCompositor.h>

using namespace Louvre;

LDNDIconRole::LDNDIconRole(Louvre::LDNDIconRole::Params *params) : LBaseSurfaceRole(params->surface->resource(), params->surface, LSurface::Role::DNDIcon)
{
    m_imp = new LDNDIconRolePrivate();

    surface()->imp()->receiveInput = false;
}

LDNDIconRole::~LDNDIconRole()
{
    if(surface())
        surface()->imp()->setMapped(false);

    delete m_imp;
}

const LPoint &LDNDIconRole::hotspotS() const
{
    return m_imp->currentHotspotS;
}

const LPoint &LDNDIconRole::hotspotC() const
{
    return m_imp->currentHotspotC;
}

const LPoint &LDNDIconRole::hotspotB() const
{
    return m_imp->currentHotspotB;
}

LDNDIconRole::LDNDIconRolePrivate *LDNDIconRole::imp() const
{
    return m_imp;
}

void LDNDIconRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    imp()->pendingHotspotOffsetS = LPoint(x,y);
}

void LDNDIconRole::handleSurfaceCommit()
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

