#include <CZ/Louvre/Protocols/Wayland/GDataDeviceManager.h>
#include <CZ/Louvre/Protocols/Wayland/RDataDevice.h>
#include <CZ/Louvre/Protocols/Wayland/RDataOffer.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Seat/LDNDSession.h>
#include <CZ/Louvre/Seat/LClipboard.h>
#include <CZ/Louvre/Seat/LSeat.h>

using namespace CZ::Protocols::Wayland;

static const struct wl_data_source_interface imp
{
    .offer = &RDataSource::offer,
    .destroy = &RDataSource::destroy,
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    .set_actions = &RDataSource::set_actions,
#endif
};

RDataSource::RDataSource
(
    GDataDeviceManager *dataDeviceManagerRes,
    UInt32 id
) noexcept
    :LResource
    (
        dataDeviceManagerRes->client(),
        &wl_data_source_interface,
        dataDeviceManagerRes->version(),
        id,
        &imp
    )
{}

RDataSource::~RDataSource() noexcept
{
    if (seat()->clipboard()->m_dataSource == this)
    {
        seat()->clipboard()->clear();

        // Save persistent MIME types
        for (auto &mimeType : m_mimeTypes)
            if (mimeType.tmp != NULL)
                seat()->clipboard()->m_persistentMimeTypes.push_back(mimeType);

        // Update current offer
        if (seat()->clipboard()->m_dataOffer && seat()->clipboard()->m_dataOffer->dataDeviceRes())
            seat()->clipboard()->m_dataOffer->dataDeviceRes()->createOffer(RDataSource::Clipboard);
    }
}

void RDataSource::requestPersistentMimeType(LClipboard::MimeTypeFile &mimeType)
{
    if (mimeType.tmp != NULL)
        return;

    if (seat()->clipboard()->persistentMimeTypeFilter(mimeType.mimeType))
    {
        mimeType.tmp = tmpfile();
        send(mimeType.mimeType.c_str(), fileno(mimeType.tmp));
    }
}

/******************** REQUESTS ********************/

void RDataSource::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RDataSource::offer(wl_client */*client*/, wl_resource *resource, const char *mime_type)
{
    auto &dataSourceRes { *static_cast<RDataSource*>(wl_resource_get_user_data(resource)) };
    dataSourceRes.m_mimeTypes.emplace_back(mime_type);

    if (&dataSourceRes == seat()->clipboard()->m_dataSource)
    {
        dataSourceRes.requestPersistentMimeType(dataSourceRes.m_mimeTypes.back());

        if (seat()->clipboard()->m_dataOffer)
            seat()->clipboard()->m_dataOffer->offer(mime_type);
    }
}

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataSource::set_actions(wl_client */*client*/, wl_resource *resource, UInt32 dnd_actions) noexcept
{
    auto &dataSourceRes { *static_cast<RDataSource*>(wl_resource_get_user_data(resource)) };

    if (dataSourceRes.usage() == RDataSource::Clipboard)
    {
        dataSourceRes.postError(WL_DATA_SOURCE_ERROR_INVALID_SOURCE, "Source usage is not DND.");
        return;
    }

    dnd_actions &= LDND::Copy | LDND::Move | LDND::Ask;

    if (dataSourceRes.actions() == dnd_actions)
        return;

    dataSourceRes.m_actions = dnd_actions;

    if (dataSourceRes.m_dndSession)
        dataSourceRes.m_dndSession->updateActions();
}
#endif

/******************** EVENTS ********************/

void RDataSource::target(const char *mimeType) noexcept
{
    wl_data_source_send_target(resource(), mimeType);
}

void RDataSource::send(const char *mimeType, Int32 fd) noexcept
{
    wl_data_source_send_send(resource(), mimeType, fd);
}

void RDataSource::cancelled() noexcept
{
    wl_data_source_send_cancelled(resource());
}

bool RDataSource::dndDropPerformed() noexcept
{
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        wl_data_source_send_dnd_drop_performed(resource());
        return true;
    }
#endif
    return false;
}

bool RDataSource::dndFinished() noexcept
{
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        wl_data_source_send_dnd_finished(resource());
        return true;
    }
#endif
    return false;
}

bool RDataSource::action(UInt32 dndAction) noexcept
{
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        wl_data_source_send_action(resource(), dndAction);
        return true;
    }
#endif
    CZ_UNUSED(dndAction);
    return false;
}
