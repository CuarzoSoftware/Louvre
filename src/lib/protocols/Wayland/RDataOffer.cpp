#include <private/LDataOfferPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LDNDManagerPrivate.h>

#include <protocols/Wayland/private/RDataOfferPrivate.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataSource.h>

#include <LClient.h>
#include <LCompositor.h>
#include <LSeat.h>

#include <cstring>
#include <stdio.h>
#include <unistd.h>

using namespace Louvre::Protocols::Wayland;

struct wl_data_offer_interface dataOffer_implementation =
{
   .accept = &RDataOffer::RDataOfferPrivate::accept,
   .receive = &RDataOffer::RDataOfferPrivate::receive,
   .destroy = &RDataOffer::RDataOfferPrivate::destroy,
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
   .finish = &RDataOffer::RDataOfferPrivate::finish,
   .set_actions = &RDataOffer::RDataOfferPrivate::set_actions
#endif
};

RDataOffer::RDataOffer(RDataDevice *dataDeviceResource, UInt32 id) :
    LResource(dataDeviceResource->client(),
              &wl_data_offer_interface,
              dataDeviceResource->version(),
              id,
              &dataOffer_implementation,
              &RDataOffer::RDataOfferPrivate::resource_destroy)
{
    m_imp = new RDataOfferPrivate();
    imp()->dataDeviceResource = dataDeviceResource;
    imp()->dataOffer = new LDataOffer(this);
}

RDataOffer::~RDataOffer()
{
    delete m_imp;
}

void RDataOffer::sendAction(UInt32 action)
{
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3)
        wl_data_offer_send_action(resource(), action);
#endif
}

void RDataOffer::sendSourceActions(UInt32 actions)
{
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3 && client()->seat()->dndManager()->source()->dataSourceResource()->version() >= 3)
        wl_data_offer_send_source_actions(resource(), client()->seat()->dndManager()->source()->dndActions());
#endif
}

void RDataOffer::sendOffer(const char *mimeType)
{
    wl_data_offer_send_offer(resource(), mimeType);
}

LDataOffer *RDataOffer::dataOffer() const
{
    return imp()->dataOffer;
}

RDataDevice *RDataOffer::dataDeviceResource() const
{
    return imp()->dataDeviceResource;
}

