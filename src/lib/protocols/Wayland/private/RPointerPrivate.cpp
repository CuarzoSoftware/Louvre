#include <protocols/Wayland/private/RPointerPrivate.h>
#include <private/LCursorRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LCompositor.h>

using namespace Louvre;

void RPointer::RPointerPrivate::resource_destroy(wl_resource *resource)
{
    RPointer *rPointer = (RPointer*)wl_resource_get_user_data(resource);
    delete rPointer;
}

void RPointer::RPointerPrivate::set_cursor(wl_client *client, wl_resource *resource, UInt32 serial, wl_resource *surface, Int32 hotspot_x, Int32 hotspot_y)
{
    L_UNUSED(client);
    RPointer *rPointer = (RPointer*)wl_resource_get_user_data(resource);

    if (serial != rPointer->serials().enter)
        return;

    if (!seat()->pointer()->focusSurface() ||
        seat()->pointer()->focusSurface()->client() != rPointer->client())
        return;

    if (surface)
    {
        Wayland::RSurface *rSurface = (Wayland::RSurface*)wl_resource_get_user_data(surface);
        LSurface *lSurface = rSurface->surface();

        if (lSurface->roleId() != LSurface::Role::Undefined && lSurface->roleId() != LSurface::Role::Cursor)
        {
            wl_resource_post_error(resource, WL_POINTER_ERROR_ROLE, "Given wl_surface has another role.");
            return;
        }

        LCursorRole::Params cursorRoleParams;
        cursorRoleParams.surface = lSurface;

        LCursorRole *lCursor = compositor()->createCursorRoleRequest(&cursorRoleParams);
        lCursor->imp()->currentHotspotS.setX(hotspot_x);
        lCursor->imp()->currentHotspotS.setY(hotspot_y);
        lCursor->imp()->currentHotspotC = lCursor->imp()->currentHotspotS * compositor()->globalScale();
        lCursor->imp()->currentHotspotB = lCursor->imp()->currentHotspotS * lSurface->bufferScale();
        lSurface->imp()->setPendingRole(lCursor);
        lSurface->imp()->applyPendingRole();
        seat()->pointer()->setCursorRequest(lCursor);
    }
    else
        seat()->pointer()->setCursorRequest(nullptr);
}

#if LOUVRE_WL_SEAT_VERSION >= 3
void RPointer::RPointerPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
