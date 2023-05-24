#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <protocols/Wayland/GDataDeviceManager.h>

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
    .offer = &RDataSource::RDataSourcePrivate::offer,
    .destroy = &RDataSource::RDataSourcePrivate::destroy,
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= WL_DATA_SOURCE_ACTION_SINCE_VERSION
    .set_actions = &RDataSource::RDataSourcePrivate::set_actions,
#endif
};

RDataSource::RDataSource(GDataDeviceManager *dataDeviceManagerGlobal, UInt32 id) :
    LResource(dataDeviceManagerGlobal->client(),
              &wl_data_source_interface,
              dataDeviceManagerGlobal->version(),
              id,
              &dataSource_implementation,
              &RDataSource::RDataSourcePrivate::resource_destroy)
{
    m_imp = new RDataSourcePrivate();
    imp()->dataSource = new LDataSource(this);
}

RDataSource::~RDataSource()
{
    // Check if being used by a Drag & Drop
    if (dataSource() == client()->seat()->dndManager()->source())
    {
        client()->seat()->dndManager()->cancel();
    }

    // Check if used by clipboard
    if (dataSource() != client()->seat()->dataSelection())
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

void RDataSource::sendDNDDropPerformed()
{
    #if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        if (compositor()->seat()->dndManager()->imp()->matchedMimeType)
            wl_data_source_send_dnd_drop_performed(resource());
        else
            compositor()->seat()->dndManager()->cancel();
    }
#endif
}

void RDataSource::sendCancelled()
{
    wl_data_source_send_cancelled(resource());
}

void RDataSource::sendAction(UInt32 action)
{
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3)
        wl_data_source_send_action(resource(), action);
#endif
}

void RDataSource::sendSend(const char *mimeType, Int32 fd)
{
    wl_data_source_send_send(resource(), mimeType, fd);
}

LDataSource *RDataSource::dataSource() const
{
    return imp()->dataSource;
}

