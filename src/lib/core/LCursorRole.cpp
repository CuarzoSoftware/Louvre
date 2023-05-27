#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LCursorRolePrivate.h>
#include <private/LSurfacePrivate.h>

#include <LCursorRole.h>
#include <LSurface.h>
#include <LCompositor.h>
#include <LCursor.h>

using namespace Louvre;

LCursorRole::LCursorRole(Params *params) : LBaseSurfaceRole(params->surface->surfaceResource(), params->surface, LSurface::Role::Cursor)
{
    m_imp = new LCursorRolePrivate();

    surface()->imp()->receiveInput = false;
}

LCursorRole::~LCursorRole()
{
    if (surface())
        surface()->imp()->setMapped(false);

    delete m_imp;
}

const LPoint &LCursorRole::hotspotS() const
{
    return imp()->currentHotspotS;
}

const LPoint &LCursorRole::hotspotC() const
{
    return imp()->currentHotspotC;
}

const LPoint &LCursorRole::hotspotB() const
{
    return imp()->currentHotspotB;
}

void LCursorRole::handleSurfaceCommit()
{
    imp()->currentHotspotS -= imp()->pendingHotspotOffsetS;
    imp()->pendingHotspotOffsetS = 0;
    imp()->currentHotspotC = imp()->currentHotspotS * compositor()->globalScale();
    imp()->currentHotspotB = imp()->currentHotspotS * surface()->bufferScale();
    hotspotChanged();

    if (surface()->buffer())
    {
        surface()->imp()->setMapped(true);

        // Notify that the cursor changed content
        if (compositor()->cursor()->texture() == surface()->texture())
        {
            surface()->seat()->pointer()->setCursorRequest(this);
        }
    }
    else
    {
        surface()->imp()->setMapped(false);
    }
}

void LCursorRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    imp()->pendingHotspotOffsetS = LPoint(x,y);
}

void LCursorRole::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);

    // Hotspot
    imp()->currentHotspotC = imp()->currentHotspotS * newScale;
}
