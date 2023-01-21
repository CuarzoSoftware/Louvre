#include <private/LClientPrivate.h>

#include <globals/Wayland/DataDeviceManager.h>
#include <globals/Wayland/DataSource.h>
#include <globals/Wayland/DataDevice.h>

#include <LCompositor.h>
#include <LDataSource.h>
#include <LDataDevice.h>

using namespace Louvre::Globals;

struct wl_data_source_interface dataSource_implementation =
{
    .offer = &DataSource::offer,
    .destroy = &DataSource::destroy,
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    .set_actions = &DataSource::set_actions,
#endif
};

struct wl_data_device_interface dataDevice_implementation =
{
    .start_drag = &DataDevice::start_drag,
    .set_selection = &DataDevice::set_selection,
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 2
    .release = &DataDevice::release
#endif
};

struct wl_data_device_manager_interface dataDeviceManager_implementation =
{
    .create_data_source = &DataDeviceManager::create_data_source,
    .get_data_device = &DataDeviceManager::get_data_device
};

void DataDeviceManager::resource_destroy(wl_resource *)
{
    // Nothing to do here
}

void DataDeviceManager::create_data_source(wl_client *client, wl_resource *resource, UInt32 id)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);
    UInt32 version = wl_resource_get_version(resource);
    wl_resource *dataSource = wl_resource_create(client, &wl_data_source_interface, version, id);
    LDataSource *lDataSource = new LDataSource(dataSource,lClient);
    wl_resource_set_implementation(dataSource, &dataSource_implementation, lDataSource, &DataSource::resource_destroy);
}
void DataDeviceManager::get_data_device(wl_client *client, wl_resource *resource, UInt32 id, wl_resource */*seat*/)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);
    LSeat *lSeat = lClient->compositor()->seat();

    if(lClient->imp()->dataDevice)
    {
        // Already got a data device
        return;
    }

    UInt32 version = wl_resource_get_version(resource);
    wl_resource *dataDevice = wl_resource_create(client, &wl_data_device_interface, version, id);
    LDataDevice *lDataDevice = new LDataDevice(dataDevice, lClient, lSeat);
    lClient->imp()->dataDevice = lDataDevice;
    wl_resource_set_implementation(dataDevice, &dataDevice_implementation, lDataDevice, &DataDevice::resource_destroy);
}

void DataDeviceManager::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    LCompositor *lCompositor = (LCompositor*)data;

    LClient *lClient = nullptr;

    // Search for the client object
    for(LClient *c : lCompositor->clients())
    {
        if(c->client() == client)
        {
            lClient = c;
            break;
        }
    }

    if(!lClient)
        return;

    wl_resource *resource = wl_resource_create(client, &wl_data_device_manager_interface, version, id);
    wl_resource_set_implementation(resource, &dataDeviceManager_implementation, lClient, &DataDeviceManager::resource_destroy);
}
