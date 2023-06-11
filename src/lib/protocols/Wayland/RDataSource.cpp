#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <protocols/Wayland/GDataDeviceManager.h>
#include <private/LDataSourcePrivate.h>
#include <private/LCompositorPrivate.h>

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
        &dataSource_implementation,
        &RDataSource::RDataSourcePrivate::resource_destroy
    )
{
    m_imp = new RDataSourcePrivate();
    imp()->lDataSource = new LDataSource(this);
}

RDataSource::~RDataSource()
{
    // Check if being used by a Drag & Drop
    if (dataSource() == seat()->dndManager()->source())
        seat()->dndManager()->cancel();

    // Check if used by clipboard
    if (dataSource() != seat()->dataSelection())
        delete imp()->lDataSource;
    else
        dataSource()->imp()->dataSourceResource = nullptr;

    delete m_imp;
}

LDataSource *RDataSource::dataSource() const
{
    return imp()->lDataSource;
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
