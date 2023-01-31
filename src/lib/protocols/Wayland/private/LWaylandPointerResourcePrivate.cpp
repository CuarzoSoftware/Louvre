#include "LWaylandPointerResourcePrivate.h"
#include <private/LCursorRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <protocols/Wayland/LWaylandSeatGlobal.h>
#include <LCompositor.h>
#include <LClient.h>


using namespace Louvre;

void LWaylandPointerResource::LWaylandPointerResourcePrivate::resource_destroy(wl_resource *resource)
{
    LWaylandPointerResource *pointerResource = (LWaylandPointerResource*)wl_resource_get_user_data(resource);
    delete pointerResource;
}

void LWaylandPointerResource::LWaylandPointerResourcePrivate::set_cursor(wl_client *, wl_resource *resource, UInt32 serial, wl_resource *surface, Int32 hotspot_x, Int32 hotspot_y)
{
    LWaylandPointerResource *pointerResource = (LWaylandPointerResource*)wl_resource_get_user_data(resource);

    if(serial != pointerResource->seatGlobal()->pointerSerials().enter)
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

        LCursorRole *lCursor = pointerResource->compositor()->createCursorRoleRequest(&cursorRoleParams);
        lCursor->imp()->currentHotspotS.setX(hotspot_x);
        lCursor->imp()->currentHotspotS.setY(hotspot_y);
        lCursor->imp()->currentHotspotC = lCursor->imp()->currentHotspotS*lSurface->compositor()->globalScale();
        lCursor->imp()->currentHotspotB = lCursor->imp()->currentHotspotS*lSurface->bufferScale();
        lSurface->imp()->setPendingRole(lCursor);
        lSurface->imp()->applyPendingRole();
        pointerResource->compositor()->seat()->pointer()->setCursorRequest(lCursor);
    }
    else
    {
        pointerResource->client()->seat()->pointer()->setCursorRequest(nullptr);
    }
}

#if LOUVRE_SEAT_VERSION >= WL_POINTER_RELEASE_SINCE_VERSION
    void LWaylandPointerResource::LWaylandPointerResourcePrivate::release(wl_client *, wl_resource *resource)
    {
        wl_resource_destroy(resource);
    }
#endif
