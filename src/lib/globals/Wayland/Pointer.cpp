#include <globals/Wayland/Pointer.h>

#include <private/LCursorRolePrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSurfacePrivate.h>

#include <LCompositor.h>
#include <LSeat.h>
#include <LPoint.h>
#include <LPointer.h>

#include <stdio.h>

using namespace Louvre::Globals;

void Pointer::resource_destroy(wl_resource *resource)
{
    LClient *client = (LClient*)wl_resource_get_user_data(resource);
    client->imp()->pointerResource = nullptr;
}

void Pointer::set_cursor(wl_client *, wl_resource *resource, UInt32 serial, wl_resource *surface, Int32 hotspot_x, Int32 hotspot_y)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);

    if(serial != lClient->seat()->pointer()->lastPointerEnterEventSerial())
        return;

    if(surface)
    {
        LSurface *lSurface = (LSurface*)wl_resource_get_user_data(surface);

        if(lSurface->roleId() != LSurface::Role::Undefined && lSurface->roleId() != LSurface::Role::Cursor)
        {
            wl_resource_post_error(resource,WL_POINTER_ERROR_ROLE,"Given wl_surface has another role.");
            return;
        }

        LCursorRole::Params cursorRoleParams;
        cursorRoleParams.surface = lSurface;

        LCursorRole *lCursor = lClient->compositor()->createCursorRoleRequest(&cursorRoleParams);
        lCursor->imp()->currentHotspotS.setX(hotspot_x);
        lCursor->imp()->currentHotspotS.setY(hotspot_y);
        lCursor->imp()->currentHotspotC = lCursor->imp()->currentHotspotS*lSurface->compositor()->globalScale();
        lCursor->imp()->currentHotspotB = lCursor->imp()->currentHotspotS*lSurface->bufferScale();
        lSurface->imp()->setPendingRole(lCursor);
        lSurface->imp()->applyPendingRole();
        lClient->compositor()->seat()->pointer()->setCursorRequest(lCursor);
    }
    else
    {
        lClient->seat()->pointer()->setCursorRequest(nullptr);
    }
}

#if LOUVRE_SEAT_VERSION >= 3
void Pointer::release(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}
#endif
