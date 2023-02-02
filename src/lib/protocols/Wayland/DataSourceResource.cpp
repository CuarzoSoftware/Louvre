#include <protocols/Wayland/private/DataSourceResourcePrivate.h>
#include <protocols/Wayland/DataDeviceManagerGlobal.h>

#include <private/LDataSourcePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LDNDManagerPrivate.h>

#include <LClient.h>
#include <LSeat.h>

#include <string.h>
#include <unistd.h>

using namespace Louvre::Protocols::Wayland;

struct wl_data_source_interface dataSource_implementation =
{
    .offer = &DataSourceResource::DataSourceResourcePrivate::offer,
    .destroy = &DataSourceResource::DataSourceResourcePrivate::destroy,
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= WL_DATA_SOURCE_ACTION_SINCE_VERSION
    .set_actions = &DataSourceResource::DataSourceResourcePrivate::set_actions,
#endif
};

DataSourceResource::DataSourceResource(DataDeviceManagerGlobal *dataDeviceManagerGlobal, UInt32 id) :
    LResource(dataDeviceManagerGlobal->client(),
              &wl_data_source_interface,
              dataDeviceManagerGlobal->version(),
              id,
              &dataSource_implementation,
              &DataSourceResource::DataSourceResourcePrivate::resource_destroy)
{
    m_imp = new DataSourceResourcePrivate();
    imp()->dataSource = new LDataSource(this);
}

DataSourceResource::~DataSourceResource()
{
    // Check if being used by a Drag & Drop
    if(dataSource() == client()->seat()->dndManager()->source())
    {
        client()->seat()->dndManager()->cancel();
    }

    // Check if used by clipboard
    if(dataSource() != client()->seat()->dataSelection())
    {
        dataSource()->imp()->remove();
        delete imp()->dataSource;
    }
    else
    {
        dataSource()->imp()->dataSourceResource = nullptr;
    }

    delete m_imp;
}

void DataSourceResource::sendDNDDropPerformed()
{
    #if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if(version() >= 3)
    {
        if(compositor()->seat()->dndManager()->imp()->matchedMimeType)
            wl_data_source_send_dnd_drop_performed(resource());
        else
            compositor()->seat()->dndManager()->cancel();
    }
#endif
}

void DataSourceResource::sendCancelled()
{
    wl_data_source_send_cancelled(resource());
}

void DataSourceResource::sendAction(UInt32 action)
{
    wl_data_source_send_action(resource(), action);
}

void DataSourceResource::sendSend(const char *mimeType, Int32 fd)
{
    wl_data_source_send_send(resource(), mimeType, fd);
}

LDataSource *DataSourceResource::dataSource() const
{
    return imp()->dataSource;
}

DataSourceResource::DataSourceResourcePrivate *DataSourceResource::imp() const
{
    return m_imp;
}
