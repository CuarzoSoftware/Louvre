#include <CZ/Louvre/Private/LDNDIconRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Cursor/LCursor.h>

using namespace CZ;

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

LDNDIconRole::~LDNDIconRole() noexcept
{
    validateDestructor();
    notifyDestruction();
}

void LDNDIconRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    m_pending.offset.set(x, y);
}

void LDNDIconRole::cacheCommit() noexcept
{
    if (surface()->isLocked())
        m_cache.emplace_back(m_pending);
}

void LDNDIconRole::applyCommit() noexcept
{
    const auto prevHotspotB { m_hotspotB };

    if (m_cache.empty())
    {
        m_current = m_pending;
        m_pending = {};
    }
    else
    {
        m_current = m_cache.front();
        m_cache.pop_front();
    }

    m_hotspot -= m_current.offset;
    m_hotspotB.set(
        m_hotspot.x() * surface()->scale(),
        m_hotspot.y() * surface()->scale());

    if (prevHotspotB != m_hotspotB)
        hotspotChanged();
    surface()->imp()->setMapped(surface()->bufferResource());
}
