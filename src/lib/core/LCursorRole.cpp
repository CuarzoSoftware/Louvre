#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LCursorRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LClientPrivate.h>
#include <LDND.h>
#include <LCursorRole.h>
#include <LClientCursor.h>
#include <LSurface.h>
#include <LCompositor.h>
#include <LCursor.h>
#include <LSeat.h>

using namespace Louvre;

LCursorRole::LCursorRole(const void *params) :
    LBaseSurfaceRole(((Params*)params)->surface->surfaceResource(),
                       ((Params*)params)->surface,
                       LSurface::Role::Cursor),
    LPRIVATE_INIT_UNIQUE(LCursorRole)
{
    surface()->imp()->stateFlags.remove(LSurface::LSurfacePrivate::ReceiveInput);
}

LCursorRole::~LCursorRole()
{
    notifyDestruction();

    if (cursor()->clientCursor() && cursor()->clientCursor()->cursorRole() == this)
        cursor()->useDefault();

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

        if (cursor()->clientCursor() && cursor()->clientCursor()->cursorRole() == this)
            cursor()->setCursor(*cursor()->clientCursor());
    }
    else
    {
        if (cursor()->clientCursor() && cursor()->clientCursor()->cursorRole() == this)
            cursor()->useDefault();

        surface()->imp()->setMapped(false);
    }
}

void LCursorRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    imp()->pendingHotspotOffset.setX(x);
    imp()->pendingHotspotOffset.setY(y);
}
