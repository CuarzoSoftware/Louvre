#include <CZ/Louvre/Private/LDNDIconRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/LSurface.h>
#include <CZ/Louvre/LCursor.h>

using namespace Louvre;

LDNDIconRole::LDNDIconRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        static_cast<const Params*>(params)->surface->surfaceResource(),
        static_cast<const Params*>(params)->surface,
        LSurface::Role::DNDIcon)
{
    surface()->imp()->stateFlags.remove(LSurface::LSurfacePrivate::ReceiveInput);

    for (LOutput *output : cursor()->intersectedOutputs())
        surface()->sendOutputEnterEvent(output);
}

LDNDIconRole::~LDNDIconRole()
{
    validateDestructor();
    notifyDestruction();
}

void LDNDIconRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    m_pendingHotspotOffset.set(x, y);
}

void LDNDIconRole::handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin)
{
    L_UNUSED(origin);
    m_currentHotspot -= m_pendingHotspotOffset;
    m_pendingHotspotOffset.set(0, 0);
    m_currentHotspotB.set(
        m_currentHotspot.x() * surface()->bufferScale(),
        m_currentHotspot.y() * surface()->bufferScale());
    hotspotChanged();
    surface()->imp()->setMapped(surface()->hasBuffer());
}
