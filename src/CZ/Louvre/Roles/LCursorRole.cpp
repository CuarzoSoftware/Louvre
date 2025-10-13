#include <CZ/Louvre/Private/LCursorRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LPointerPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/Cursor/LRoleCursorSource.h>
#include <CZ/Louvre/Cursor/LImageCursorSource.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LSeat.h>

using namespace CZ;

LCursorRole::LCursorRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        static_cast<const Params*>(params)->surface->surfaceResource(),
        static_cast<const Params*>(params)->surface,
        LSurface::Role::Cursor)
{
    surface()->imp()->stateFlags.remove(LSurface::LSurfacePrivate::ReceiveInput);

    for (LOutput *output : cursor()->intersectedOutputs())
        surface()->sendOutputEnterEvent(output);

    m_cursor = LRoleCursorSource::MakeDefault(client());
    m_cursor->m_surface = surface();
}

LCursorRole::~LCursorRole() noexcept
{
    validateDestructor();

    if (cursor()->source() == m_cursor)
    {
        cursor()->setSource({});
        cursor()->setVisible(true);
    }
}

void LCursorRole::applyCommit() noexcept
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

    m_cursor->m_visibility = surface()->mapped() ? LCursorSource::Visible : LCursorSource::Hidden;
    m_cursor->m_hotspot = m_hotspotB;

    if (surface()->image())
        m_cursor->m_image = surface()->image();

    if (cursor()->source() == m_cursor)
        cursor()->setSource(m_cursor);
}

void LCursorRole::cacheCommit() noexcept
{
    if (surface()->isLocked())
        m_cache.emplace_back(m_pending);
}

void LCursorRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    m_pending.offset.set(x, y);
}
