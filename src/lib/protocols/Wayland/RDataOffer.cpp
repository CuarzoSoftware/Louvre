#include <protocols/Wayland/private/RDataOfferPrivate.h>
#include <protocols/Wayland/RDataDevice.h>
#include <private/LDataOfferPrivate.h>

using namespace Louvre::Protocols::Wayland;

struct wl_data_offer_interface dataOffer_implementation =
{
   .accept = &RDataOffer::RDataOfferPrivate::accept,
   .receive = &RDataOffer::RDataOfferPrivate::receive,
   .destroy = &RDataOffer::RDataOfferPrivate::destroy,
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
   .finish = &RDataOffer::RDataOfferPrivate::finish,
   .set_actions = &RDataOffer::RDataOfferPrivate::set_actions
#endif
};

RDataOffer::RDataOffer
(
    RDataDevice *rDataDevice,
    UInt32 id
)
    :LResource
    (
        rDataDevice->client(),
        &wl_data_offer_interface,
        rDataDevice->version(),
        id,
        &dataOffer_implementation,
        &RDataOffer::RDataOfferPrivate::resource_destroy
    ),
    LPRIVATE_INIT_UNIQUE(RDataOffer)
{
    imp()->rDataDevice = rDataDevice;
    imp()->lDataOffer = new LDataOffer(this);
}

RDataOffer::~RDataOffer()
{
    delete imp()->lDataOffer;
}

LDataOffer *RDataOffer::dataOffer() const
{
    return imp()->lDataOffer;
}

RDataDevice *RDataOffer::dataDeviceResource() const
{
    return imp()->rDataDevice;
}

bool RDataOffer::offer(const char *mimeType)
{
    wl_data_offer_send_offer(resource(), mimeType);
    return true;
}

bool RDataOffer::sourceActions(UInt32 sourceActions)
{
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        wl_data_offer_send_source_actions(resource(), sourceActions);
        return true;
    }
#endif
    L_UNUSED(sourceActions);
    return false;
}

bool RDataOffer::action(UInt32 dndAction)
{
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    if (version() >= 3)
    {
        wl_data_offer_send_action(resource(), dndAction);
        return true;
    }
#endif
    L_UNUSED(dndAction);
    return false;
}
