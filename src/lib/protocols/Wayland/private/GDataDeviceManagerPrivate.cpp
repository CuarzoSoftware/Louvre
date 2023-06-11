#include <protocols/Wayland/private/GDataDeviceManagerPrivate.h>
#include <protocols/Wayland/RDataSource.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LClientPrivate.h>
#include <LDataSource.h>
#include <LDataDevice.h>
#include <LCompositor.h>
#include <LLog.h>

struct wl_data_device_manager_interface dataDeviceManager_implementation =
{
    .create_data_source = &GDataDeviceManager::GDataDeviceManagerPrivate::create_data_source,
    .get_data_device = &GDataDeviceManager::GDataDeviceManagerPrivate::get_data_device
};

void GDataDeviceManager::GDataDeviceManagerPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data);
    LClient *lClient = compositor()->getClientFromNativeResource(client);

    if (lClient->dataDeviceManagerGlobal())
    {
        LLog::warning("Client bound twice to the wl_data_device_manager singleton global. Ignoring it...");
        return;
    }

    new GDataDeviceManager(
        lClient,
        &wl_data_device_manager_interface,
        version,
        id,
        &dataDeviceManager_implementation,
        &GDataDeviceManager::GDataDeviceManagerPrivate::resource_destroy);
}

void GDataDeviceManager::GDataDeviceManagerPrivate::resource_destroy(wl_resource *resource)
{
    GDataDeviceManager *gDataDeviceManager = (GDataDeviceManager*)wl_resource_get_user_data(resource);
    delete gDataDeviceManager;
}

void GDataDeviceManager::GDataDeviceManagerPrivate::create_data_source(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);
    GDataDeviceManager *gDataDeviceManager = (GDataDeviceManager*)wl_resource_get_user_data(resource);
    new RDataSource(gDataDeviceManager, id);
}

void GDataDeviceManager::GDataDeviceManagerPrivate::get_data_device(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *seat)
{
    L_UNUSED(client);
    GSeat *gSeat = (GSeat*)wl_resource_get_user_data(seat);

    if (gSeat->dataDeviceResource())
    {
        LLog::warning("Client already created a wl_data_device for this wl_seat. Ignoring it.");
        return;
    }

    GDataDeviceManager *gDataDeviceManager = (GDataDeviceManager*)wl_resource_get_user_data(resource);
    new RDataDevice(gDataDeviceManager, gSeat, id);
}
