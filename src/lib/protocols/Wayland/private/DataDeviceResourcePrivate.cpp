#include "DataDeviceResourcePrivate.h"
#include <private/LDataSourcePrivate.h>
#include <LCompositor.h>
#include <private/LDNDIconRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LSeatPrivate.h>
#include <protocols/Wayland/DataSourceResource.h>
#include <protocols/Wayland/SeatGlobal.h>
#include <protocols/Wayland/PointerResource.h>
#include <protocols/Wayland/KeyboardResource.h>

void DataDeviceResource::DataDeviceResourcePrivate::resource_destroy(wl_resource *resource)
{
    DataDeviceResource *lDataDeviceResource = (DataDeviceResource*)wl_resource_get_user_data(resource);
    delete lDataDeviceResource;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 2
void DataDeviceResource::DataDeviceResourcePrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif

void DataDeviceResource::DataDeviceResourcePrivate::start_drag(wl_client *client,
                                                               wl_resource *resource,
                                                               wl_resource *source,
                                                               wl_resource *origin,
                                                               wl_resource *icon,
                                                               UInt32 serial)
{
    L_UNUSED(client);

    DataDeviceResource *lDataDeviceResource = (DataDeviceResource*)wl_resource_get_user_data(resource);
    LDNDManager *dndManager = lDataDeviceResource->compositor()->seat()->dndManager();

    /*
    // Check grab serial and if there is a current dragging goin on
    if(dndManager->dragging()
            || !lDataDeviceResource->seatGlobal()->pointerResource()
            || (lDataDeviceResource->seatGlobal()->pointerResource()->serials().button != serial))
        return; */

    // Removes pevious data source if any
    dndManager->cancel();

    // Check if there is an icon
    if(icon)
    {
        LSurface *lIcon = (LSurface*)wl_resource_get_user_data(icon);

        if(!(lIcon->roleId() == LSurface::Role::Undefined || lIcon->roleId() == LSurface::Role::DNDIcon))
        {
            wl_resource_post_error(resource,WL_DATA_DEVICE_ERROR_ROLE,"Given wl_surface has another role.");
            return;
        }

        LDNDIconRole::Params dndIconRoleParams;
        dndIconRoleParams.surface = lIcon;

        lIcon->imp()->setPendingRole(lIcon->compositor()->createDNDIconRoleRequest(&dndIconRoleParams));
        lIcon->imp()->applyPendingRole();
        lIcon->imp()->mapped = true;

        dndManager->imp()->icon = lIcon->dndIcon();

    }
    else
    {
        dndManager->imp()->icon = nullptr;
    }

    LSurface *lOrigin = (LSurface*)wl_resource_get_user_data(origin);

    dndManager->imp()->origin = lOrigin;

    // If source is null all drag events are sent only to the origin surface
    if(source)
    {
        DataSourceResource *lDataSourceResource = (DataSourceResource*)wl_resource_get_user_data(source);

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
        // Check if DND action was set
        if(lDataSourceResource->version() >= 3 && lDataSourceResource->dataSource()->dndActions() == DND_NO_ACTION_SET)
        {
            dndManager->imp()->icon = nullptr;
            return;
        }
#endif

        dndManager->imp()->source = lDataSourceResource->dataSource();
    }
    else
    {
        dndManager->imp()->source = nullptr;
    }

    // Notify
    dndManager->startDragRequest();

    if(dndManager->seat()->pointer()->focusSurface())
    {
        dndManager->seat()->pointer()->focusSurface()->client()->dataDevice().imp()->sendDNDEnterEvent(
                    dndManager->seat()->pointer()->focusSurface(),0,0);
    }
}

void DataDeviceResource::DataDeviceResourcePrivate::set_selection(wl_client *, wl_resource *resource, wl_resource *source, UInt32 serial)
{
    DataDeviceResource *lDataDeviceResource = (DataDeviceResource*)wl_resource_get_user_data(resource);
    LSeat *lSeat = lDataDeviceResource->client()->seat();

    if(source)
    {
        DataSourceResource *lDataSourceResource = (DataSourceResource*)wl_resource_get_user_data(source);

        // If this source is already used for clipboard
        if(lDataSourceResource->dataSource() == lSeat->dataSelection())
            return;

        // Check if serial matches any event
        // if(lDataDeviceResource->seatGlobal()->keyboardResource()->serials().key != serial &&
                // lDataDeviceResource->seatGlobal()->pointerResource()->serials().button != serial)
            // return;

        // Ask the developer if the client should set the clipboard
        if(!lSeat->setSelectionRequest(&lDataDeviceResource->client()->dataDevice()))
        {
            lDataSourceResource->sendCancelled();
            return;
        }

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
        // Check if was prevously used for DND
        if(lDataSourceResource->version() >= 3 && lDataSourceResource->dataSource()->dndActions() != DND_NO_ACTION_SET)
        {
            wl_resource_post_error(lDataSourceResource->resource(), WL_DATA_SOURCE_ERROR_INVALID_SOURCE, "Source for selection was previusly used for DND.");
            return;
        }
#endif

        // Delete previous selected data source if already destroyed by client
        if(lSeat->dataSelection())
        {
            if(lSeat->dataSelection()->dataSourceResource())
                lSeat->dataSelection()->dataSourceResource()->sendCancelled();
            else
            {
                lSeat->dataSelection()->imp()->remove();
                delete lSeat->imp()->dataSelection;
            }
        }

        // Mark current source as selected
        lSeat->imp()->dataSelection = lDataSourceResource->dataSource();

        // Ask client to write to the compositor fds
        for(const LDataSource::LSource &s : lDataSourceResource->dataSource()->sources())
            lDataSourceResource->sendSend(s.mimeType, fileno(s.tmp));

    }

    // A NULL source should unset the clipboard, but we keep a copy of the contents

    // If a client already has keyboard focus, send it the current clipboard
    if(lSeat->keyboard()->focusSurface())
    {
        lSeat->keyboard()->focusSurface()->client()->dataDevice().sendSelectionEvent();
    }
}
