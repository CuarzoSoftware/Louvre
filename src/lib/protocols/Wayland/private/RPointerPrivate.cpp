#include <protocols/Wayland/private/RPointerPrivate.h>
#include <private/LCursorRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LClientPrivate.h>
#include <LDNDManager.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LCursor.h>
#include <LLog.h>

using namespace Louvre;

void RPointer::RPointerPrivate::set_cursor(wl_client *client, wl_resource *resource, UInt32 serial, wl_resource *surface, Int32 hotspot_x, Int32 hotspot_y)
{
    L_UNUSED(client);

    const RPointer *rPointer { (RPointer*)wl_resource_get_user_data(resource) };
    const LClient *lClient { rPointer->client() };

    if (lClient->events().pointer.enter.serial() != serial)
    {
        LLog::warning("[RPointer::RPointerPrivate::set_cursor] Set cursor request without valid pointer enter event serial. Ignoring it.");
        return;
    }

    if (surface)
    {
        const Wayland::RSurface *rSurface { (Wayland::RSurface*)wl_resource_get_user_data(surface) };
        LSurface *lSurface { rSurface->surface() };

        if (lSurface->imp()->pending.role || (lSurface->roleId() != LSurface::Role::Undefined && lSurface->roleId() != LSurface::Role::Cursor))
        {
            wl_resource_post_error(resource, WL_POINTER_ERROR_ROLE, "Given wl_surface has another role.");
            return;
        }

        LCursorRole::Params cursorRoleParams;
        cursorRoleParams.surface = lSurface;
        LCursorRole *lCursor { compositor()->createCursorRoleRequest(&cursorRoleParams) };
        lCursor->imp()->currentHotspot.setX(hotspot_x);
        lCursor->imp()->currentHotspot.setY(hotspot_y);
        lCursor->imp()->currentHotspotB = lCursor->imp()->currentHotspot * lSurface->bufferScale();
        lSurface->imp()->setPendingRole(lCursor);
        lSurface->imp()->applyPendingRole();

        if (&lClient->imp()->lastCursorRequest == cursor()->clientCursor())
            cursor()->useDefault();

        lClient->imp()->lastCursorRequest.m_role.reset(lCursor);
        lClient->imp()->lastCursorRequest.m_triggeringEvent = lClient->events().pointer.enter;
        lClient->imp()->lastCursorRequest.m_visible = true;
        seat()->pointer()->setCursorRequest(lClient->imp()->lastCursorRequest);
        return;
    }

    if (&lClient->imp()->lastCursorRequest == cursor()->clientCursor())
        cursor()->useDefault();

    lClient->imp()->lastCursorRequest.m_role.reset();
    lClient->imp()->lastCursorRequest.m_triggeringEvent = lClient->events().pointer.enter;
    lClient->imp()->lastCursorRequest.m_visible = false;
    seat()->pointer()->setCursorRequest(lClient->imp()->lastCursorRequest);
}

#if LOUVRE_WL_SEAT_VERSION >= 3
void RPointer::RPointerPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
