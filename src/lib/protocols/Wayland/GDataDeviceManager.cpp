#include <protocols/Wayland/GDataDeviceManager.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataSource.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LLog.h>

using namespace Louvre::Protocols::Wayland;

static const struct wl_data_device_manager_interface imp
{
    .create_data_source = &GDataDeviceManager::create_data_source,
    .get_data_device = &GDataDeviceManager::get_data_device
};

void GDataDeviceManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GDataDeviceManager(client, version, id);
}

void GDataDeviceManager::create_data_source(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    new RDataSource(static_cast<GDataDeviceManager*>(wl_resource_get_user_data(resource)), id);
}

void GDataDeviceManager::get_data_device(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *seat) noexcept
{
    GSeat *seatRes { static_cast<GSeat*>(wl_resource_get_user_data(seat)) };

    if (seatRes->dataDeviceRes())
    {
        LLog::warning("[GDataDeviceManager::get_data_device] Client already created a wl_data_device for this wl_seat. Ignoring it.");
        return;
    }

    new RDataDevice(static_cast<GDataDeviceManager*>(wl_resource_get_user_data(resource)), seatRes, id);
}

GDataDeviceManager::GDataDeviceManager
(
    wl_client *client,
    Int32 version,
    UInt32 id) noexcept
    :LResource
    (
        client,
        &wl_data_device_manager_interface,
        version,
        id,
        &imp
    )
{
    this->client()->imp()->dataDeviceManagerGlobals.push_back(this);
}

GDataDeviceManager::~GDataDeviceManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->dataDeviceManagerGlobals, this);
}
