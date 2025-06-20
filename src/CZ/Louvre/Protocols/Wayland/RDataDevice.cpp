#include <CZ/Louvre/Protocols/Wayland/GDataDeviceManager.h>
#include <CZ/Louvre/Protocols/Wayland/RDataDevice.h>
#include <CZ/Louvre/Protocols/Wayland/RDataOffer.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Private/LDNDIconRolePrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/LKeyboard.h>
#include <CZ/Louvre/LClipboard.h>
#include <CZ/Louvre/LDNDSession.h>
#include <CZ/Louvre/LDND.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/LLog.h>

using namespace Louvre::Protocols::Wayland;

static const struct wl_data_device_interface imp
{
    .start_drag = &RDataDevice::start_drag,
    .set_selection = &RDataDevice::set_selection,
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 2
    .release = &RDataDevice::release
#endif
};

RDataDevice::RDataDevice
(
    GDataDeviceManager *dataDeviceManagerRes,
    GSeat *seatRes,
    Int32 id
) noexcept
    :LResource
    (
        seatRes->client(),
        &wl_data_device_interface,
        dataDeviceManagerRes->version(),
        id,
        &imp
    ),
    m_seatRes(seatRes)
{
    seatRes->m_dataDeviceRes.reset(this);
}

RDataOffer *RDataDevice::createOffer(RDataSource::Usage usage) noexcept
{
    LClipboard &clipboard { *seat()->clipboard() };

    if (usage == RDataSource::Clipboard)
    {
        if (clipboard.mimeTypes().empty())
        {
            selection(nullptr);
            return nullptr;
        }

        RDataOffer *offer { new RDataOffer(this, 0, RDataSource::Clipboard) };
        clipboard.m_dataOffer.reset(offer);
        dataOffer(offer);

        for (const auto &mimeType : seat()->clipboard()->mimeTypes())
            offer->offer(mimeType.mimeType.c_str());

        selection(offer);

        return offer;
    }
    else if (usage == RDataSource::DND)
    {
        if (!seat()->dnd()->m_session || !seat()->dnd()->m_session->source)
            return nullptr;

        RDataOffer *offer { new RDataOffer(this, 0, RDataSource::DND) };
        dataOffer(offer);

        for (const auto &mimeType : seat()->dnd()->m_session->source->m_mimeTypes)
            offer->offer(mimeType.mimeType.c_str());

        return offer;
    }

    return nullptr;
}

/******************** REQUESTS ********************/

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 2
void RDataDevice::release(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
#endif

void RDataDevice::start_drag(wl_client */*client*/,
                             wl_resource *resource,
                             wl_resource *source,
                             wl_resource *origin,
                             wl_resource *icon,
                             UInt32 serial)
{
    const LEvent *triggeringEvent;
    LDNDSession *session = new LDNDSession();
    auto *res { static_cast<RDataDevice*>(wl_resource_get_user_data(resource)) };
    session->origin.reset(static_cast<RSurface*>(wl_resource_get_user_data(origin))->surface());
    session->srcDataDevice.reset(res);

    if (source)
    {
        session->source.reset(static_cast<RDataSource*>(wl_resource_get_user_data(source)));

        if (session->source->usage() != RDataSource::Undefined)
        {
            res->postError(WL_DATA_DEVICE_ERROR_USED_SOURCE, "Source already used.");
            goto fail;
        }

        session->source->m_usage = RDataSource::DND;
    }

    if (seat()->dnd()->dragging())
    {
        LLog::warning("[RDataDevicePrivate::start_drag] Start drag request cancelled. There already is an active drag & drop session.");
        goto fail;
    }

    triggeringEvent = session->srcDataDevice->client()->findEventBySerial(serial);

    if (!triggeringEvent)
    {
        LLog::warning("[RDataDevicePrivate::start_drag] Start drag & drop request without serial match. Ignoring it.");
        goto fail;
    }

    // Check if there is an icon
    if (icon)
    {
        LSurface *iconSurface { static_cast<RSurface*>(wl_resource_get_user_data(icon))->surface() };

        if (!iconSurface->dndIcon())
        {
            if (!iconSurface->imp()->canHostRole())
            {
                res->postError(WL_DATA_DEVICE_ERROR_ROLE, "Given wl_surface has another role.");
                goto fail;
            }

            LDNDIconRole::Params dndIconRoleParams;
            dndIconRoleParams.surface = iconSurface;
            iconSurface->imp()->setLayer(LLayerOverlay);
            LFactory::createObject<LDNDIconRole>(&dndIconRoleParams);
            iconSurface->imp()->notifyRoleChange();
            session->icon.reset(iconSurface->dndIcon());
        }
    }

    // Check if DND action was set
    if (session->source && session->source->version() >= 3 && session->source->actions() == 0)
        goto fail;

    seat()->dnd()->m_triggeringEvent.reset(triggeringEvent->copy());
    seat()->dnd()->m_session.reset(session);

    if (session->source)
        session->source->m_dndSession = seat()->dnd()->m_session;

    seat()->dnd()->startDragRequest();
    return;

fail:
    if (session->source)
        session->source->cancelled();

    if (session->icon)
        session->icon->surface()->imp()->setMapped(false);

    delete session;
}

void RDataDevice::set_selection(wl_client *client, wl_resource *resource, wl_resource *source, UInt32 serial)
{
    L_UNUSED(client);

    RDataDevice *rDataDevice { static_cast<RDataDevice*>(wl_resource_get_user_data(resource)) };

    const LEvent *triggeringEvent { rDataDevice->client()->findEventBySerial(serial) };

    if (!triggeringEvent)
    {
        LLog::warning("[RDataDevicePrivate::set_selection] Set clipboard request without valid triggering event. Ignoring it.");
        return;
    }

    if (source)
    {
        RDataSource *rDataSource { static_cast<RDataSource*>(wl_resource_get_user_data(source)) };

        if (seat()->clipboard()->m_dataSource == rDataSource)
        {
            LLog::warning("[RDataDevicePrivate::set_selection] Set clipboard request already made with the same data source. Ignoring it.");
            return;
        }

        if (rDataSource->usage() != RDataSource::Undefined)
        {
            rDataSource->postError(WL_DATA_DEVICE_ERROR_USED_SOURCE, "Source already used.");
            return;
        }

        // Ask the user if the client should set the clipboard
        if (!seat()->clipboard()->setClipboardRequest(rDataDevice->client(), *triggeringEvent))
        {
            LLog::debug("[RDataDevicePrivate::set_selection] Set clipboard request denied by user.");
            return;
        }

        rDataSource->m_usage = RDataSource::Clipboard;

        if (seat()->clipboard()->m_dataSource)
            seat()->clipboard()->m_dataSource->cancelled();

        seat()->clipboard()->m_dataSource.reset();
        seat()->clipboard()->clear();

        // Ask client to write to the compositor fds
        for (auto &mimeType : rDataSource->m_mimeTypes)
            rDataSource->requestPersistentMimeType(mimeType);

        seat()->clipboard()->m_dataSource.reset(rDataSource);

        // If a client already has keyboard focus, send it the current clipboard
        if (seat()->keyboard()->focus())
        {
            LClient &client { *seat()->keyboard()->focus()->client() };

            for (auto *seat : client.seatGlobals())
            {
                if (seat->dataDeviceRes())
                {
                    seat->dataDeviceRes()->createOffer(RDataSource::Clipboard);
                    break;
                }
            }
        }
    }
    else
    {
        /* A NULL source should unset the clipboard, but we keep a copy of the contents
         * then we don't let clients unset the clipboard. */
    }
}

/******************** EVENTS ********************/

void RDataDevice::dataOffer(RDataOffer *offer) noexcept
{
    wl_data_device_send_data_offer(resource(), offer->resource());
}

void RDataDevice::enter(UInt32 serial, RSurface *surface, Float24 x, Float24 y, RDataOffer *offer) noexcept
{
    wl_data_device_send_enter(resource(),
                              serial,
                              surface->resource(),
                              x, y,
                              offer == nullptr ? NULL : offer->resource());
}

void RDataDevice::leave() noexcept
{
    wl_data_device_send_leave(resource());
}

void RDataDevice::motion(UInt32 time, Float24 x, Float24 y) noexcept
{
    wl_data_device_send_motion(resource(), time, x, y);
}

void RDataDevice::drop() noexcept
{
    wl_data_device_send_drop(resource());
}

void RDataDevice::selection(RDataOffer *offer) noexcept
{
    wl_data_device_send_selection(resource(), offer == nullptr ? NULL : offer->resource());
}
