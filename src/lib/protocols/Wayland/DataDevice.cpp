#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LDNDIconRolePrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LSeatPrivate.h>

#include <protocols/Wayland/DataDevice.h>
#include <protocols/Wayland/DataOffer.h>

#include <LDataOffer.h>
#include <LKeyboard.h>
#include <LPointer.h>

#include <stdio.h>

using namespace Louvre::Globals;

void DataDevice::resource_destroy(wl_resource *resource)
{
    LDataDevice *lDataDevice = (LDataDevice*)wl_resource_get_user_data(resource);
    lDataDevice->client()->imp()->dataDevice = nullptr;
    delete lDataDevice;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 2
void DataDevice::release(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}
#endif

void DataDevice::start_drag(wl_client *, wl_resource *resource, wl_resource *source, wl_resource *origin, wl_resource *icon, UInt32 /*serial*/)
{
    LDataDevice *lDataDevice = (LDataDevice*)wl_resource_get_user_data(resource);

    // Check grab serial and if there is a current dragging goin on
    if(lDataDevice->seat()->dndManager()->dragging())
        return;

    // Removes pevious data source if any
    lDataDevice->seat()->dndManager()->cancel();

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

        lDataDevice->seat()->dndManager()->imp()->icon = lIcon->dndIcon();

    }
    else
    {
        lDataDevice->seat()->dndManager()->imp()->icon = nullptr;
    }

    LSurface *lOrigin = (LSurface*)wl_resource_get_user_data(origin);

    lDataDevice->seat()->dndManager()->imp()->origin = lOrigin;

    // If source is null all drag events are sent only to the origin surface
    if(source)
    {
        LDataSource *lDataSource = (LDataSource*)wl_resource_get_user_data(source);

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
        // Check if DND action was set
        if(wl_resource_get_version(lDataSource->resource()) >= 3 && lDataSource->dndActions() == DND_NO_ACTION_SET)
        {
            lDataDevice->seat()->dndManager()->imp()->icon = nullptr;
            return;
        }
#endif

        lDataDevice->seat()->dndManager()->imp()->source = lDataSource;
    }
    else
    {
        lDataDevice->seat()->dndManager()->imp()->source = nullptr;
    }

    // Notify
    lDataDevice->seat()->dndManager()->startDragRequest();

    if(lDataDevice->seat()->pointer()->focusSurface() && lDataDevice->seat()->pointer()->focusSurface()->client()->dataDevice())
    {
        lDataDevice->seat()->pointer()->focusSurface()->client()->dataDevice()->imp()->sendDNDEnterEvent(lDataDevice->seat()->pointer()->focusSurface(),0,0);
    }
}

void DataDevice::set_selection(wl_client *, wl_resource *resource, wl_resource *source, UInt32 /*serial*/)
{
    LDataDevice *lDataDevice = (LDataDevice*)wl_resource_get_user_data(resource);

    if(source != NULL)
    {
        LDataSource *lDataSource = (LDataSource*)wl_resource_get_user_data(source);

        if(lDataSource == lDataDevice->seat()->dataSelection())
            return;

        if(!lDataDevice->seat()->setSelectionRequest(lDataDevice))
        {
            wl_data_source_send_cancelled(lDataSource->resource());
            return;
        }

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
        // Check if was prevously used for DND
        if(wl_resource_get_version(lDataSource->resource()) >= 3 && lDataSource->dndActions() != DND_NO_ACTION_SET)
        {
            wl_resource_post_error(lDataSource->resource(),WL_DATA_SOURCE_ERROR_INVALID_SOURCE,"Source for selection was previusly used for DND.");
            return;
        }
#endif

        // Delete previous selected data source if already destroyed by client
        if(lDataDevice->seat()->dataSelection())
        {
            if(lDataDevice->seat()->dataSelection()->resource())
                wl_data_source_send_cancelled(lDataDevice->seat()->dataSelection()->resource());
            else
            {
                lDataDevice->seat()->dataSelection()->imp()->remove();
                delete lDataDevice->seat()->imp()->dataSelection;
            }

        }

        // Mark current source as selected
        lDataDevice->seat()->imp()->dataSelection = lDataSource;

        // Ask client to write to the compositor fds
        for(const LDataSource::LSource &s : lDataSource->sources())
            wl_data_source_send_send(lDataSource->resource(),s.mimeType,fileno(s.tmp));


    }

    if(lDataDevice->client()->compositor()->seat()->keyboard()->focusSurface() &&
       lDataDevice->client()->compositor()->seat()->keyboard()->focusSurface()->client()->dataDevice())
    {
        lDataDevice->client()->compositor()->seat()->keyboard()->focusSurface()->client()->dataDevice()->sendSelectionEvent();
    }
}


