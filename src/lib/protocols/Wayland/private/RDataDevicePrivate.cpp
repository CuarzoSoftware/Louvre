#include <protocols/Wayland/private/RDataDevicePrivate.h>
#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <private/LDNDIconRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LCompositorPrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RPointer.h>
#include <protocols/Wayland/RKeyboard.h>
#include <LCompositor.h>
#include <LClient.h>
#include <LLog.h>
#include <string.h>
#include <fcntl.h>
#include <LKeyboard.h>
#include <LClipboard.h>

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
    /* TODO
    L_UNUSED(client);

    RDataSource *rDataSource { nullptr };
    RDataDevice *rDataDevice { (RDataDevice*)wl_resource_get_user_data(resource) };
    RSurface *rOriginSurface { (RSurface*)wl_resource_get_user_data(origin) };
    LDNDManager &dndManager { *seat()->dndManager() };

    if (source)
        rDataSource = (RDataSource*)wl_resource_get_user_data(source);

    // Cancel if already dragging
    if (dndManager.dragging())
    {
        if (rDataSource)
            rDataSource->cancelled();
        return;
    }

    const LEvent *event { rDataDevice->client()->findEventBySerial(serial) };

    if (!event)
    {
        LLog::warning("[RDataDevicePrivate::start_drag] Start drag & drop request without serial match. Ignoring it.");

        if (rDataSource)
            rDataSource->cancelled();

        return;
    }

    dndManager.imp()->dropped = false;

    // Removes pevious data source if any
    dndManager.cancel();

    dndManager.imp()->triggeringEvent.reset(event->copy());

    // Check if there is an icon
    if (icon)
    {
        RSurface *rSurface { (RSurface*)wl_resource_get_user_data(icon) };
        LSurface *lIcon { rSurface->surface() };

        if (lIcon->imp()->pending.role || (lIcon->roleId() != LSurface::Role::Undefined && lIcon->roleId() != LSurface::Role::DNDIcon))
        {
            dndManager.cancel();
            wl_resource_post_error(resource, WL_DATA_DEVICE_ERROR_ROLE, "Given wl_surface has another role.");
            return;
        }

        // Retry if the compositor surfaces list changes
    retry:
        compositor()->imp()->surfacesListChanged = false;
        for (LSurface *s : compositor()->surfaces())
            if (s->dndIcon())
            {
                s->imp()->setMapped(false);

                if (compositor()->imp()->surfacesListChanged)
                    goto retry;
            }

        LDNDIconRole::Params dndIconRoleParams;
        dndIconRoleParams.surface = lIcon;
        lIcon->imp()->setPendingRole(compositor()->createDNDIconRoleRequest(&dndIconRoleParams));
        lIcon->imp()->applyPendingRole();
        lIcon->imp()->stateFlags.add(LSurface::LSurfacePrivate::Mapped);
        dndManager.imp()->icon = lIcon->dndIcon();
    }
    else
        dndManager.imp()->icon = nullptr;

    dndManager.imp()->origin = rOriginSurface->surface();

    // If source is null all drag events are sent only to the origin surface
    if (source)
    {
        RDataSource *rDataSource { (RDataSource*)wl_resource_get_user_data(source) };

        // Check if DND action was set
        if (rDataSource->version() >= 3 && rDataSource->dataSource()->dndActions() == LOUVRE_DND_NO_ACTION_SET)
        {
            dndManager.cancel();
            return;
        }

        dndManager.imp()->source = rDataSource->dataSource();
    }
    else
        dndManager.imp()->source = nullptr;

    dndManager.imp()->srcDataDevice = rDataDevice;

    // Notify
    dndManager.startDragRequest();
    */
}

void RDataDevice::RDataDevicePrivate::set_selection(wl_client *client, wl_resource *resource, wl_resource *source, UInt32 serial)
{
    L_UNUSED(client);

    RDataDevice *rDataDevice { static_cast<RDataDevice*>(wl_resource_get_user_data(resource)) };

    if (source)
    {
        RDataSource *rDataSource { static_cast<RDataSource*>(wl_resource_get_user_data(source)) };

        if (seat()->clipboard()->m_dataSource.get() == rDataSource)
        {
            LLog::warning("[RDataDevicePrivate::set_selection] Set clipboard request already made with the same data source. Ignoring it.");
            return;
        }

        if (rDataSource->usage() != RDataSource::Undefined)
        {
            wl_resource_post_error(rDataSource->resource(), WL_DATA_DEVICE_ERROR_USED_SOURCE, "Source already used.");
            return;
        }

        const LEvent *triggeringEvent { rDataDevice->client()->findEventBySerial(serial) };

        if (!triggeringEvent)
            LLog::warning("[RDataDevicePrivate::set_selection] Set clipboard request without valid triggering event. Letting the user decide...");

        // Ask the user if the client should set the clipboard
        if (!seat()->clipboard()->setClipboardRequest(rDataDevice->client(), triggeringEvent))
        {
            LLog::debug("[RDataDevicePrivate::set_selection] Set clipboard request denied by user.");
            return;
        }

        rDataSource->imp()->usage = RDataSource::Clipboard;
        seat()->clipboard()->m_dataSource.reset();
        seat()->clipboard()->clear();

        // Ask client to write to the compositor fds
        for (auto &mimeType : rDataSource->imp()->mimeTypes)
            rDataSource->requestPersistentMimeType(mimeType);

        seat()->clipboard()->m_dataSource.reset(rDataSource);

        // If a client already has keyboard focus, send it the current clipboard
        if (seat()->keyboard()->focus())
        {
            LClient &client { *seat()->keyboard()->focus()->client() };

            for (auto *seat : client.seatGlobals())
            {
                if (seat->dataDeviceResource())
                {
                    seat->dataDeviceResource()->createOffer(RDataSource::Clipboard);
                    break;
                }
            }
        }
    }
    else
    {
        // A NULL source should unset the clipboard, but we keep a copy of the contents
        // then we don't let clients unset the clipboard.
    }
}
