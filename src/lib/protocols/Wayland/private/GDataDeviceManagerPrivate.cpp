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

void GDataDeviceManager::GDataDeviceManagerPrivate::bind(wl_client *client, void *compositor, UInt32 version, UInt32 id)
{
    LCompositor *lCompositor = (LCompositor*)compositor;

    LClient *lClient = lCompositor->getClientFromNativeResource(client);

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
    GDataDeviceManager *lGDataDeviceManager = (GDataDeviceManager*)wl_resource_get_user_data(resource);
    delete lGDataDeviceManager;
}

void GDataDeviceManager::GDataDeviceManagerPrivate::create_data_source(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);

    GDataDeviceManager *lGDataDeviceManager = (GDataDeviceManager*)wl_resource_get_user_data(resource);
    new RDataSource(lGDataDeviceManager, id);
}
void GDataDeviceManager::GDataDeviceManagerPrivate::get_data_device(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *seat)
{
    L_UNUSED(client);

    GSeat *seatGlobal = (GSeat*)wl_resource_get_user_data(seat);

    if (seatGlobal->dataDeviceResource())
    {
        LLog::warning("Client already created a wl_data_device for this wl_seat. Ignoring it.");
        return;
    }

    GDataDeviceManager *lGDataDeviceManager = (GDataDeviceManager*)wl_resource_get_user_data(resource);
    new RDataDevice(lGDataDeviceManager, seatGlobal, id);
}
