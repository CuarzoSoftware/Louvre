#include "LTime.h"
#include "LWayland.h"
#include <protocols/Wayland/DataDeviceManagerGlobal.h>
#include <protocols/Wayland/private/SeatGlobalPrivate.h>
#include <protocols/Wayland/private/DataDeviceResourcePrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LClientPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LDNDIconRolePrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LSeatPrivate.h>

#include <protocols/Wayland/DataDeviceResource.h>
#include <protocols/Wayland/DataOfferResource.h>

#include <LDataOffer.h>
#include <LKeyboard.h>
#include <LPointer.h>

#include <stdio.h>

using namespace Louvre::Protocols::Wayland;


struct wl_data_device_interface dataDevice_implementation =
{
    .start_drag = &DataDeviceResource::DataDeviceResourcePrivate::start_drag,
    .set_selection = &DataDeviceResource::DataDeviceResourcePrivate::set_selection,
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= WL_DATA_DEVICE_RELEASE_SINCE_VERSION
    .release = &DataDeviceResource::DataDeviceResourcePrivate::release
#endif
};

DataDeviceResource::DataDeviceResource(DataDeviceManagerGlobal *dataDeviceManagerGlobal, SeatGlobal *seatGlobal, Int32 id) :
LResource(seatGlobal->client(),
          &wl_data_device_interface,
          dataDeviceManagerGlobal->version(),
          id,
          &dataDevice_implementation,
          &DataDeviceResource::DataDeviceResourcePrivate::resource_destroy)
{
    m_imp = new DataDeviceResourcePrivate();
    seatGlobal->imp()->dataDeviceResource = this;
}

DataDeviceResource::~DataDeviceResource()
{
    if(seatGlobal())
        seatGlobal()->imp()->dataDeviceResource = nullptr;

    delete m_imp;
}

void DataDeviceResource::sendEnter(LSurface *surface, Float64 x, Float64 y, DataOfferResource *dataOfferResource)
{
    wl_data_device_send_enter(resource(),
                             LWayland::nextSerial(),
                             surface->surfaceResource()->resource(),
                             wl_fixed_from_double(x),
                             wl_fixed_from_double(y),
                             dataOfferResource->resource());
}

void DataDeviceResource::sendLeave()
{
    wl_data_device_send_leave(resource());
}

void DataDeviceResource::sendMotion(Float64 x, Float64 y)
{
    wl_data_device_send_motion(resource(),
                               LTime::ms(),
                               wl_fixed_from_double(x),
                               wl_fixed_from_double(y));
}

void DataDeviceResource::sendDrop()
{
    wl_data_device_send_drop(resource());
}

void DataDeviceResource::sendDataOffer(DataOfferResource *dataOfferResource)
{
    wl_data_device_send_data_offer(resource(), dataOfferResource->resource());
}

void DataDeviceResource::sendSelection(DataOfferResource *dataOfferResource)
{
    wl_data_device_send_selection(resource(), dataOfferResource->resource());
}

SeatGlobal *DataDeviceResource::seatGlobal() const
{
    return imp()->seatGlobal;
}

LDataOffer *DataDeviceResource::dataOffered() const
{
    return imp()->dataOffered;
}

DataDeviceResource::DataDeviceResourcePrivate *DataDeviceResource::imp() const
{
    return m_imp;
}
