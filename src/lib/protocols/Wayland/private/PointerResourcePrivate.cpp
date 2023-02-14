#include "PointerResourcePrivate.h"
#include <private/LCursorRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <protocols/Wayland/SeatGlobal.h>
#include <LCompositor.h>
#include <LClient.h>

using namespace Louvre;

void PointerResource::PointerResourcePrivate::resource_destroy(wl_resource *resource)
{
    PointerResource *pointerResource = (PointerResource*)wl_resource_get_user_data(resource);
    delete pointerResource;
}

void PointerResource::PointerResourcePrivate::set_cursor(wl_client *, wl_resource *resource, UInt32 serial, wl_resource *surface, Int32 hotspot_x, Int32 hotspot_y)
{
    PointerResource *pointerResource = (PointerResource*)wl_resource_get_user_data(resource);

    if(serial != pointerResource->serials().enter)
        return;

    if(surface)
    {
        Protocols::Wayland::SurfaceResource *lSurfaceResource = (Protocols::Wayland::SurfaceResource*)wl_resource_get_user_data(surface);
        LSurface *lSurface = lSurfaceResource->surface();

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
    void PointerResource::PointerResourcePrivate::release(wl_client *, wl_resource *resource)
    {
        wl_resource_destroy(resource);
    }
#endif
