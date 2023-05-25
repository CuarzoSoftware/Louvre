#include "RDataDevicePrivate.h"
#include <private/LDataSourcePrivate.h>
#include <LCompositor.h>
#include <private/LDNDIconRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LSeatPrivate.h>
#include <protocols/Wayland/RDataSource.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RPointer.h>
#include <protocols/Wayland/RKeyboard.h>

void RDataDevice::RDataDevicePrivate::resource_destroy(wl_resource *resource)
{
    RDataDevice *lRDataDevice = (RDataDevice*)wl_resource_get_user_data(resource);
    delete lRDataDevice;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 2
void RDataDevice::RDataDevicePrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif

void RDataDevice::RDataDevicePrivate::start_drag(wl_client *client,
                                                               wl_resource *resource,
                                                               wl_resource *source,
                                                               wl_resource *origin,
                                                               wl_resource *icon,
                                                               UInt32 serial)
{
    L_UNUSED(client);

    /* TODO: Use serial. */
    L_UNUSED(serial);

    RDataDevice *lRDataDevice = (RDataDevice*)wl_resource_get_user_data(resource);
    LDNDManager *dndManager = lRDataDevice->compositor()->seat()->dndManager();

    /*
    // Check grab serial and if there is a current dragging goin on
    if (dndManager->dragging()
            || !lRDataDevice->seatGlobal()->pointerResource()
            || (lRDataDevice->seatGlobal()->pointerResource()->serials().button != serial))
        return; */

    // Removes pevious data source if any
    dndManager->cancel();

    // Check if there is an icon
    if (icon)
    {
        RSurface *lRSurface = (RSurface*)wl_resource_get_user_data(icon);
        LSurface *lIcon = lRSurface->surface();

        if (!(lIcon->roleId() == LSurface::Role::Undefined || lIcon->roleId() == LSurface::Role::DNDIcon))
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

    RSurface *lRSurface = (RSurface*)wl_resource_get_user_data(origin);
    LSurface *lOrigin = lRSurface->surface();

    dndManager->imp()->origin = lOrigin;

    // If source is null all drag events are sent only to the origin surface
    if (source)
    {
        RDataSource *lRDataSource = (RDataSource*)wl_resource_get_user_data(source);

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
        // Check if DND action was set
        if (lRDataSource->version() >= 3 && lRDataSource->dataSource()->dndActions() == DND_NO_ACTION_SET)
        {
            dndManager->imp()->icon = nullptr;
            return;
        }
#endif

        dndManager->imp()->source = lRDataSource->dataSource();
    }
    else
    {
        dndManager->imp()->source = nullptr;
    }

    // Notify
    dndManager->startDragRequest();

    if (dndManager->seat()->pointer()->focusSurface())
    {
        dndManager->seat()->pointer()->focusSurface()->client()->dataDevice().imp()->sendDNDEnterEvent(
                    dndManager->seat()->pointer()->focusSurface(),0,0);
    }
}

void RDataDevice::RDataDevicePrivate::set_selection(wl_client *, wl_resource *resource, wl_resource *source, UInt32 serial)
{
    /* TODO: Use serial. */
    L_UNUSED(serial);

    RDataDevice *lRDataDevice = (RDataDevice*)wl_resource_get_user_data(resource);
    LSeat *lSeat = lRDataDevice->client()->seat();

    if (source)
    {
        RDataSource *lRDataSource = (RDataSource*)wl_resource_get_user_data(source);

        // If this source is already used for clipboard
        if (lRDataSource->dataSource() == lSeat->dataSelection())
            return;

        // Check if serial matches any event
        // if (lRDataDevice->seatGlobal()->keyboardResource()->serials().key != serial &&
                // lRDataDevice->seatGlobal()->pointerResource()->serials().button != serial)
            // return;

        // Ask the developer if the client should set the clipboard
        if (!lSeat->setSelectionRequest(&lRDataDevice->client()->dataDevice()))
        {
            lRDataSource->sendCancelled();
            return;
        }

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
        // Check if was prevously used for DND
        if (lRDataSource->version() >= 3 && lRDataSource->dataSource()->dndActions() != DND_NO_ACTION_SET)
        {
            wl_resource_post_error(lRDataSource->resource(), WL_DATA_SOURCE_ERROR_INVALID_SOURCE, "Source for selection was previusly used for DND.");
            return;
        }
#endif

        // Delete previous selected data source if already destroyed by client
        if (lSeat->dataSelection())
        {
            if (lSeat->dataSelection()->dataSourceResource())
                lSeat->dataSelection()->dataSourceResource()->sendCancelled();
            else
            {
                lSeat->dataSelection()->imp()->remove();
                delete lSeat->imp()->dataSelection;
            }
        }

        // Mark current source as selected
        lSeat->imp()->dataSelection = lRDataSource->dataSource();

        // Ask client to write to the compositor fds
        for (const LDataSource::LSource &s : lRDataSource->dataSource()->sources())
            lRDataSource->sendSend(s.mimeType, fileno(s.tmp));

    }

    // A NULL source should unset the clipboard, but we keep a copy of the contents

    // If a client already has keyboard focus, send it the current clipboard
    if (lSeat->keyboard()->focusSurface())
    {
        lSeat->keyboard()->focusSurface()->client()->dataDevice().sendSelectionEvent();
    }
}
