#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <protocols/Wayland/GDataDeviceManager.h>
#include <protocols/Wayland/RDataDevice.h>
#include <private/LCompositorPrivate.h>
#include <LClipboard.h>
#include <LDND.h>
#include <LSeat.h>

using namespace Louvre::Protocols::Wayland;

struct wl_data_source_interface dataSource_implementation =
{
    .offer = &RDataSource::RDataSourcePrivate::offer,
    .destroy = &RDataSource::RDataSourcePrivate::destroy,
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    .set_actions = &RDataSource::RDataSourcePrivate::set_actions,
#endif
};

RDataSource::RDataSource
(
    GDataDeviceManager *gDataDeviceManager,
    UInt32 id
)
    :LResource
    (
        gDataDeviceManager->client(),
        &wl_data_source_interface,
        gDataDeviceManager->version(),
        id,
        &dataSource_implementation
    ),
    LPRIVATE_INIT_UNIQUE(RDataSource)
{}

RDataSource::~RDataSource()
{
    if (seat()->clipboard()->m_dataSource.get() == this)
    {
        seat()->clipboard()->clear();

        // Save persistent MIME types
        for (auto &mimeType : imp()->mimeTypes)
            if (mimeType.tmp != NULL)
                seat()->clipboard()->m_persistentMimeTypes.push_back(mimeType);

        // Update current offer
        if (seat()->clipboard()->m_dataOffer.get() && seat()->clipboard()->m_dataOffer.get()->dataDeviceResource())
            seat()->clipboard()->m_dataOffer.get()->dataDeviceResource()->createOffer(RDataSource::Clipboard);
    }

    /* TODO
    // Check if being used by a Drag & Drop
    if (dataSource() == seat()->dndManager()->source())
        seat()->dndManager()->cancel();

    // Check if used by clipboard
    if (dataSource() != seat()->dataSelection())
        delete imp()->lDataSource;
    else
    {
        dataSource()->imp()->removeClientOnlySources();
        dataSource()->imp()->dataSourceResource = nullptr;
    }*/
}

void RDataSource::requestPersistentMimeType(MimeTypeFile &mimeType) noexcept
{
    if (mimeType.tmp != NULL)
        return;

    if (seat()->clipboard()->persistentMimeTypeFilter(mimeType.mimeType))
    {
        mimeType.tmp = tmpfile();
        send(mimeType.mimeType.c_str(), fileno(mimeType.tmp));
    }
}

RDataSource::Usage RDataSource::usage() const noexcept
{
    return imp()->usage;
}

UInt32 RDataSource::actions() const noexcept
{
    return imp()->actions;
}

bool RDataSource::target(const char *mimeType)
{
    wl_data_source_send_target(resource(), mimeType);
    return true;
}

bool RDataSource::send(const char *mimeType, Int32 fd)
{
    wl_data_source_send_send(resource(), mimeType, fd);
    return true;
}

bool RDataSource::cancelled()
{
    wl_data_source_send_cancelled(resource());
    return true;
}

bool RDataSource::dndDropPerformed()
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

bool RDataSource::dndFinished()
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

bool RDataSource::action(UInt32 dndAction)
{
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        wl_data_source_send_action(resource(), dndAction);
        return true;
    }
#endif
    L_UNUSED(dndAction);
    return false;
}
