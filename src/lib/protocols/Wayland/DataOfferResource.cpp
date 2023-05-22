#include <private/LDataOfferPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LDNDManagerPrivate.h>

#include <protocols/Wayland/private/DataOfferResourcePrivate.h>
#include <protocols/Wayland/DataDeviceResource.h>
#include <protocols/Wayland/DataSourceResource.h>

#include <LClient.h>
#include <LCompositor.h>
#include <LSeat.h>

#include <cstring>
#include <stdio.h>
#include <unistd.h>

using namespace Louvre::Protocols::Wayland;

struct wl_data_offer_interface dataOffer_implementation =
{
   .accept = &DataOfferResource::DataOfferResourcePrivate::accept,
   .receive = &DataOfferResource::DataOfferResourcePrivate::receive,
   .destroy = &DataOfferResource::DataOfferResourcePrivate::destroy,
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
   .finish = &DataOfferResource::DataOfferResourcePrivate::finish,
   .set_actions = &DataOfferResource::DataOfferResourcePrivate::set_actions
#endif
};

DataOfferResource::DataOfferResource(DataDeviceResource *dataDeviceResource, UInt32 id) :
    LResource(dataDeviceResource->client(),
              &wl_data_offer_interface,
              dataDeviceResource->version(),
              id,
              &dataOffer_implementation,
              &DataOfferResource::DataOfferResourcePrivate::resource_destroy)
{
    m_imp = new DataOfferResourcePrivate();
    imp()->dataDeviceResource = dataDeviceResource;
    imp()->dataOffer = new LDataOffer(this);
}

DataOfferResource::~DataOfferResource()
{
    delete m_imp;
}

void DataOfferResource::sendAction(UInt32 action)
{
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if(version() >= 3)
        wl_data_offer_send_action(resource(), action);
#endif
}

void DataOfferResource::sendSourceActions(UInt32 actions)
{
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if(version() >= 3 && client()->seat()->dndManager()->source()->dataSourceResource()->version() >= 3)
        wl_data_offer_send_source_actions(resource(), client()->seat()->dndManager()->source()->dndActions());
#endif
}

void DataOfferResource::sendOffer(const char *mimeType)
{
    wl_data_offer_send_offer(resource(), mimeType);
}

LDataOffer *DataOfferResource::dataOffer() const
{
    return imp()->dataOffer;
}

DataDeviceResource *DataOfferResource::dataDeviceResource() const
{
    return imp()->dataDeviceResource;
}

DataOfferResource::DataOfferResourcePrivate *DataOfferResource::imp() const
{
    return m_imp;
}
