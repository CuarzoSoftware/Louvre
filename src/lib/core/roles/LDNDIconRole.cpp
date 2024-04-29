#include <private/LDNDIconRolePrivate.h>
#include <private/LSurfacePrivate.h>

#include <LSurface.h>
#include <LCompositor.h>

using namespace Louvre;

LDNDIconRole::LDNDIconRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        static_cast<const Params*>(params)->surface->surfaceResource(),
        static_cast<const Params*>(params)->surface,
        LSurface::Role::DNDIcon),
    LPRIVATE_INIT_UNIQUE(LDNDIconRole)
{
    surface()->imp()->stateFlags.remove(LSurface::LSurfacePrivate::ReceiveInput);
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

void LDNDIconRole::handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin)
{
    L_UNUSED(origin);

    imp()->currentHotspot -= imp()->pendingHotspotOffset;
    imp()->pendingHotspotOffset = LPoint();
    imp()->currentHotspotB = imp()->currentHotspot * surface()->bufferScale();
    hotspotChanged();

    surface()->imp()->setMapped(surface()->buffer() != nullptr);
}
