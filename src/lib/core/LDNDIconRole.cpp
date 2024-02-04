#include <private/LDNDIconRolePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>

#include <LSurface.h>
#include <LCompositor.h>

using namespace Louvre;

LDNDIconRole::LDNDIconRole(void *params) :
    LBaseSurfaceRole(((Params*)params)->surface->surfaceResource(),
                     ((Params*)params)->surface,
                    LSurface::Role::DNDIcon),
    LPRIVATE_INIT_UNIQUE(LDNDIconRole)
{
    surface()->imp()->receiveInput = false;
}

LDNDIconRole::~LDNDIconRole()
{
    if (surface())
        surface()->imp()->setMapped(false);
}

const LPoint &LDNDIconRole::hotspot() const
{
    return imp()->currentHotspot;
}

const LPoint &LDNDIconRole::hotspotB() const
{
    return imp()->currentHotspotB;
}

void LDNDIconRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    imp()->pendingHotspotOffset = LPoint(x,y);
}

void LDNDIconRole::handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin)
{
    L_UNUSED(origin);

    imp()->currentHotspot -= imp()->pendingHotspotOffset;
    imp()->pendingHotspotOffset = LPoint();
    imp()->currentHotspotB = imp()->currentHotspot * surface()->bufferScale();
    hotspotChanged();

    surface()->imp()->setMapped(surface()->buffer() != nullptr);
}
