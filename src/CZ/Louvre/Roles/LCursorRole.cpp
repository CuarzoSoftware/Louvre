#include <CZ/Louvre/Private/LCursorRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LPointerPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
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

    for (LOutput *output : cursor()->intersectedOutputs())
        surface()->sendOutputEnterEvent(output);
}

LCursorRole::~LCursorRole()
{
    validateDestructor();

    if (cursor()->clientCursor() && cursor()->clientCursor()->cursorRole() == this)
        cursor()->useDefault();
}

void LCursorRole::handleSurfaceCommit(CommitOrigin origin)
{
    L_UNUSED(origin);

    m_currentHotspot -= m_pendingHotspotOffset;
    m_pendingHotspotOffset.set(0, 0);
    m_currentHotspotB.set(
        m_currentHotspot.x() * surface()->bufferScale(),
        m_currentHotspot.y() * surface()->bufferScale());

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
    m_pendingHotspotOffset.set(x, y);
}
