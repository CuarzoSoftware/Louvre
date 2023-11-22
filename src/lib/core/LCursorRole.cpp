#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LCursorRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LPointerPrivate.h>
#include <LCursorRole.h>
#include <LSurface.h>
#include <LCompositor.h>
#include <LCursor.h>

using namespace Louvre;

LCursorRole::LCursorRole(Params *params) :
    LBaseSurfaceRole(params->surface->surfaceResource(), params->surface, LSurface::Role::Cursor),
    LPRIVATE_INIT_UNIQUE(LCursorRole)
{
    surface()->imp()->receiveInput = false;
}

LCursorRole::~LCursorRole()
{
    if (seat()->pointer()->imp()->lastCursorRequest == this)
        seat()->pointer()->imp()->lastCursorRequest = nullptr;

    if (surface())
        surface()->imp()->setMapped(false);
}

const LPoint &LCursorRole::hotspot() const
{
    return imp()->currentHotspot;
}

const LPoint &LCursorRole::hotspotB() const
{
    return imp()->currentHotspotB;
}

void LCursorRole::handleSurfaceCommit(Wayland::RSurface::CommitOrigin origin)
{
    L_UNUSED(origin);

    imp()->currentHotspot -= imp()->pendingHotspotOffset;
    imp()->pendingHotspotOffset = 0;
    imp()->currentHotspotB = imp()->currentHotspot * surface()->bufferScale();

    hotspotChanged();

    if (surface()->buffer())
    {
        surface()->imp()->setMapped(true);

        // Notify that the cursor changed content
        if (seat()->pointer()->imp()->lastCursorRequest == this && (
           (seat()->pointer()->focus() && seat()->pointer()->focus()->client() == surface()->client()) ||
           (seat()->dndManager()->origin() && seat()->dndManager()->origin()->client() == surface()->client())))
        {
            seat()->pointer()->imp()->lastCursorRequestWasHide = false;
            seat()->pointer()->setCursorRequest(this);
        }
    }
    else
        surface()->imp()->setMapped(false);
}

void LCursorRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    imp()->pendingHotspotOffset = LPoint(x,y);
}
