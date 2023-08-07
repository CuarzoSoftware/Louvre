#include "RDataDevicePrivate.h"
#include "LLog.h"
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
    RDataDevice *rDataDevice = (RDataDevice*)wl_resource_get_user_data(resource);
    delete rDataDevice;
}

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 2
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
    /* TODO: Use serial. */
    L_UNUSED(serial);
    L_UNUSED(client);

    RDataDevice *rDataDevice = (RDataDevice*)wl_resource_get_user_data(resource);
    RSurface *rOriginSurface = (RSurface*)wl_resource_get_user_data(origin);
    LSurface *lOriginSurface = rOriginSurface->surface();
    LDNDManager *dndManager = seat()->dndManager();

    // Cancel if there is dragging going on from another client or if there is no focused surface from this client
    if ( (dndManager->dragging() && dndManager->source() && dndManager->source()->client() != rDataDevice->client()) ||
        (seat()->pointer()->focusSurface() != lOriginSurface))
    {
        LLog::debug("[data device] Invalid DND drag request.");
        return;
    }

    // Removes pevious data source if any
    dndManager->cancel();

    // Check if there is an icon
    if (icon)
    {
        RSurface *rSurface = (RSurface*)wl_resource_get_user_data(icon);
        LSurface *lIcon = rSurface->surface();

        if (lIcon->imp()->pending.role ||
            (lIcon->roleId() != LSurface::Role::Undefined && lIcon->roleId() != LSurface::Role::DNDIcon))
        {
            wl_resource_post_error(resource, WL_DATA_DEVICE_ERROR_ROLE, "Given wl_surface has another role.");
            return;
        }

        for (LSurface *s : compositor()->surfaces())
            if (s->dndIcon())
                s->imp()->setMapped(false);

        LDNDIconRole::Params dndIconRoleParams;
        dndIconRoleParams.surface = lIcon;
        lIcon->imp()->setPendingRole(compositor()->createDNDIconRoleRequest(&dndIconRoleParams));
        lIcon->imp()->applyPendingRole();
        lIcon->imp()->mapped = true;
        dndManager->imp()->icon = lIcon->dndIcon();
    }
    else
        dndManager->imp()->icon = nullptr;

    dndManager->imp()->origin = lOriginSurface;

    // If source is null all drag events are sent only to the origin surface
    if (source)
    {
        RDataSource *rDataSource = (RDataSource*)wl_resource_get_user_data(source);

        // Check if DND action was set
        if (rDataSource->version() >= 3 && rDataSource->dataSource()->dndActions() == LOUVRE_DND_NO_ACTION_SET)
        {
            dndManager->cancel();
            return;
        }

        dndManager->imp()->source = rDataSource->dataSource();
    }
    else
        dndManager->imp()->source = nullptr;

    // Notify
    dndManager->startDragRequest();

    if (seat()->pointer()->focusSurface())
    {
        seat()->pointer()->focusSurface()->client()->dataDevice().imp()->sendDNDEnterEventS(
            seat()->pointer()->focusSurface(), 0, 0);
    }
}

void RDataDevice::RDataDevicePrivate::set_selection(wl_client *client, wl_resource *resource, wl_resource *source, UInt32 serial)
{
    L_UNUSED(client);

    RDataDevice *rDataDevice = (RDataDevice*)wl_resource_get_user_data(resource);

    if (source)
    {
        RDataSource *rDataSource = (RDataSource*)wl_resource_get_user_data(source);

        // If this source is already used for clipboard
        if (rDataSource->dataSource() == seat()->dataSelection())
            return;

        // Ask the developer if the client should set the clipboard
        if (!seat()->setSelectionRequest(&rDataDevice->client()->dataDevice(), serial))
        {
            rDataSource->cancelled();
            return;
        }

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
        // Check if was prevously used for DND
        if (rDataSource->version() >= 3 && rDataSource->dataSource()->dndActions() != LOUVRE_DND_NO_ACTION_SET)
        {
            wl_resource_post_error(rDataSource->resource(), WL_DATA_SOURCE_ERROR_INVALID_SOURCE, "Source for selection was previusly used for DND.");
            return;
        }
#endif

        // Delete previous selected data source if already destroyed by client
        if (seat()->dataSelection())
        {
            if (seat()->dataSelection()->dataSourceResource())
                seat()->dataSelection()->dataSourceResource()->cancelled();
            else
            {
                delete seat()->imp()->dataSelection;
            }
        }

        // Mark current source as selected
        seat()->imp()->dataSelection = rDataSource->dataSource();

        // Ask client to write to the compositor fds
        for (const LDataSource::LSource &s : rDataSource->dataSource()->sources())
            rDataSource->send(s.mimeType, fileno(s.tmp));

        // If a client already has keyboard focus, send it the current clipboard
        if (seat()->keyboard()->focusSurface())
            seat()->keyboard()->focusSurface()->client()->dataDevice().sendSelectionEvent();
    }
    else
    {
        /* A NULL source should unset the clipboard, but we keep a copy of the contents
         * then we don't let clients unset the clipboard. */
    }
}
