#include <private/LClientPrivate.h>
#include <protocols/Wayland/private/DataDeviceManagerGlobalPrivate.h>
#include <LCompositor.h>
#include <LLog.h>

using namespace Louvre::Protocols::Wayland;

struct wl_data_device_manager_interface dataDeviceManager_implementation =
{
    .create_data_source = &DataDeviceManagerGlobal::DataDeviceManagerGlobalPrivate::create_data_source,
    .get_data_device = &DataDeviceManagerGlobal::DataDeviceManagerGlobalPrivate::get_data_device
};

DataDeviceManagerGlobal::DataDeviceManagerGlobal(LClient *client,
                                                                 const wl_interface *interface,
                                                                 Int32 version,
                                                                 UInt32 id,
                                                                 const void *implementation,
                                                                 wl_resource_destroy_func_t destroy) :
    LResource(client,
              interface,
              version,
              id,
              implementation,
              destroy)
{
    m_imp = new DataDeviceManagerGlobalPrivate();
    client->imp()->dataDeviceManagerGlobal = this;

}

DataDeviceManagerGlobal::~DataDeviceManagerGlobal()
{
    client()->imp()->dataDeviceManagerGlobal = nullptr;
    delete m_imp;
}


void DataDeviceManagerGlobal::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    LCompositor *lCompositor = (LCompositor*)data;

    LClient *lClient = lCompositor->getClientFromNativeResource(client);

    if(lClient->dataDeviceManagerGlobal())
    {
        LLog::warning("Client bound twice to the wl_data_device_manager singleton global. Ignoring it.");
        return;
    }

    new DataDeviceManagerGlobal(
                lClient,
                &wl_data_device_manager_interface,
                version,
                id,
                &dataDeviceManager_implementation,
                &DataDeviceManagerGlobal::DataDeviceManagerGlobalPrivate::resource_destroy);
}


