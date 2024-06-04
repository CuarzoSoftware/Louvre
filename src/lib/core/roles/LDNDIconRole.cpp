#include <private/LDNDIconRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LSurface.h>

using namespace Louvre;

LDNDIconRole::LDNDIconRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        static_cast<const Params*>(params)->surface->surfaceResource(),
        static_cast<const Params*>(params)->surface,
        LSurface::Role::DNDIcon)
{
    surface()->imp()->stateFlags.remove(LSurface::LSurfacePrivate::ReceiveInput);
}

LDNDIconRole::~LDNDIconRole()
{
    if (surface())
        surface()->imp()->setMapped(false);
}

void LDNDIconRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    m_pendingHotspotOffset = LPoint(x,y);
}

void LDNDIconRole::handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin)
{
    L_UNUSED(origin);

    m_currentHotspot -= m_pendingHotspotOffset;
    m_pendingHotspotOffset = LPoint();
    m_currentHotspotB = m_currentHotspot * surface()->bufferScale();
    hotspotChanged();

    surface()->imp()->setMapped(surface()->hasBuffer());
}
