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
#include <LDNDSession.h>

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
    L_UNUSED(client);

    const LEvent *triggeringEvent;
    LDNDSession *session = new LDNDSession();
    session->origin.reset(static_cast<RSurface*>(wl_resource_get_user_data(origin))->surface());
    session->srcDataDevice.reset(static_cast<RDataDevice*>(wl_resource_get_user_data(resource)));

    if (source)
    {
        session->source.reset(static_cast<RDataSource*>(wl_resource_get_user_data(source)));

        if (session->source.get()->usage() != RDataSource::Undefined)
        {
            wl_resource_post_error(resource, WL_DATA_DEVICE_ERROR_USED_SOURCE, "Source already used.");
            goto fail;
        }

        session->source.get()->imp()->usage = RDataSource::DND;
    }

    if (seat()->dnd()->dragging())
    {   
        LLog::warning("[RDataDevicePrivate::start_drag] Start drag request cancelled. There already is an active drag & drop session.");
        goto fail;
    }

    triggeringEvent = session->srcDataDevice.get()->client()->findEventBySerial(serial);

    if (!triggeringEvent)
    {
        LLog::warning("[RDataDevicePrivate::start_drag] Start drag & drop request without serial match. Ignoring it.");
        goto fail;
    }

    // Check if there is an icon
    if (icon)
    {
        LSurface *iconSurface { static_cast<RSurface*>(wl_resource_get_user_data(icon))->surface() };

        if (iconSurface->imp()->pending.role || (iconSurface->roleId() != LSurface::Role::Undefined && iconSurface->roleId() != LSurface::Role::DNDIcon))
        {
            wl_resource_post_error(resource, WL_DATA_DEVICE_ERROR_ROLE, "Given wl_surface has another role.");
            goto fail;
        }

        LDNDIconRole::Params dndIconRoleParams;
        dndIconRoleParams.surface = iconSurface;
        iconSurface->imp()->setPendingRole(compositor()->createDNDIconRoleRequest(&dndIconRoleParams));
        iconSurface->imp()->applyPendingRole();
        iconSurface->imp()->stateFlags.add(LSurface::LSurfacePrivate::Mapped);
        session->icon.reset(iconSurface->dndIcon());
    }

    // Check if DND action was set
    if (session->source.get() && session->source.get()->version() >= 3 && session->source.get()->actions() == 0)
        goto fail;

    seat()->dnd()->m_triggeringEvent.reset(triggeringEvent->copy());
    seat()->dnd()->m_session.reset(session);

    if (session->source.get())
        session->source.get()->imp()->dndSession = seat()->dnd()->m_session;

    seat()->dnd()->startDragRequest();
    return;

    fail:
    if (session->source.get())
        session->source.get()->cancelled();

    if (session->icon.get())
        session->icon.get()->surface()->imp()->setMapped(false);

    delete session;
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
