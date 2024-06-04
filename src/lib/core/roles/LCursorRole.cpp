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

LCursorRole::LCursorRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        static_cast<const Params*>(params)->surface->surfaceResource(),
        static_cast<const Params*>(params)->surface,
        LSurface::Role::Cursor)
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

void LCursorRole::handleSurfaceCommit(CommitOrigin origin)
{
    L_UNUSED(origin);

    m_currentHotspot -= m_pendingHotspotOffset;
    m_pendingHotspotOffset = 0;
    m_currentHotspotB = m_currentHotspot * surface()->bufferScale();

    hotspotChanged();

    surface()->imp()->setMapped(surface()->hasBuffer());

    if (cursor()->clientCursor() && cursor()->clientCursor()->cursorRole() == this)
    {
        client()->imp()->lastCursorRequest.m_visible = surface()->mapped();
        cursor()->setCursor(*cursor()->clientCursor());
    }
}

void LCursorRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    m_pendingHotspotOffset.setX(x);
    m_pendingHotspotOffset.setY(y);
}
