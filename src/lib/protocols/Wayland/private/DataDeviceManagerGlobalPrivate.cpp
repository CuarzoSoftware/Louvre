#include "DataDeviceManagerGlobalPrivate.h"
#include <protocols/Wayland/DataSourceResource.h>
#include <protocols/Wayland/DataDeviceResource.h>
#include <protocols/Wayland/SeatGlobal.h>
#include <LDataSource.h>
#include <LDataDevice.h>
#include <private/LClientPrivate.h>
#include <LLog.h>

using namespace Louvre::Globals;

void DataDeviceManagerGlobal::DataDeviceManagerGlobalPrivate::resource_destroy(wl_resource *resource)
{
    DataDeviceManagerGlobal *lDataDeviceManagerGlobal = (DataDeviceManagerGlobal*)wl_resource_get_user_data(resource);
    delete lDataDeviceManagerGlobal;
}

void DataDeviceManagerGlobal::DataDeviceManagerGlobalPrivate::create_data_source(wl_client *client, wl_resource *resource, UInt32 id)
{
    L_UNUSED(client);
    DataDeviceManagerGlobal *lDataDeviceManagerGlobal = (DataDeviceManagerGlobal*)wl_resource_get_user_data(resource);
    new DataSourceResource(lDataDeviceManagerGlobal, id);
}
void DataDeviceManagerGlobal::DataDeviceManagerGlobalPrivate::get_data_device(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *seat)
{
    L_UNUSED(client);
    SeatGlobal *seatGlobal = (SeatGlobal*)wl_resource_get_user_data(seat);

    if(seatGlobal->dataDeviceResource())
    {
        LLog::warning("Client already created a wl_data_device for this wl_seat. Ignoring it.");
        return;
    }

    DataDeviceManagerGlobal *lDataDeviceManagerGlobal = (DataDeviceManagerGlobal*)wl_resource_get_user_data(resource);
    new DataDeviceResource(lDataDeviceManagerGlobal, seatGlobal, id);
}
