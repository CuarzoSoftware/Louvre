#include <private/LDataDevicePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LDNDManagerPrivate.h>

#include <globals/Wayland/DataOffer.h>

#include <LWayland.h>
#include <LTime.h>

using namespace Louvre;

struct wl_data_offer_interface dataOffer_implementation =
{
   .accept = &Globals::DataOffer::accept,
   .receive = &Globals::DataOffer::receive,
   .destroy = &Globals::DataOffer::destroy,
#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
   .finish = &Globals::DataOffer::finish,
   .set_actions = &Globals::DataOffer::set_actions
#endif
};


LDataDevice::LDataDevice(wl_resource *resource, LClient *client, LSeat *seat)
{
    m_imp = new LDataDevicePrivate();
    m_imp->dataDevice = this;
    m_imp->seat = seat;
    m_imp->resource = resource;
    m_imp->client = client;
}

LDataDevice::~LDataDevice()
{
    delete m_imp;
}

wl_resource *LDataDevice::resource() const
{
    return m_imp->resource;
}

LClient *LDataDevice::client() const
{
    return m_imp->client;
}

LSeat *LDataDevice::seat() const
{
    return m_imp->seat;
}

LDataOffer *LDataDevice::dataOffered() const
{
    return m_imp->dataOffered;
}

UInt32 LDataDevice::lastDataOfferId() const
{
    return m_imp->lastDataOfferId;
}

void LDataDevice::sendSelectionEvent()
{
    // Send data device selection first
    if(seat()->dataSelection())
    {
        wl_resource *dataOffer = wl_resource_create(client()->client(),&wl_data_offer_interface,wl_resource_get_version(resource()),0);

        if(dataOffer != NULL)
        {
            LDataOffer *lDataOffer = new LDataOffer(dataOffer,this);
            wl_resource_set_implementation(dataOffer, &dataOffer_implementation, lDataOffer, &Globals::DataOffer::resource_destroy);
            lDataOffer->imp()->usedFor = LDataOffer::Selection;
            wl_data_device_send_data_offer(resource(),dataOffer);

            for(const LDataSource::LSource &s : seat()->dataSelection()->sources())
                wl_data_offer_send_offer(dataOffer,s.mimeType);


            wl_data_device_send_selection(resource(),dataOffer);
        }
    }

}

void LDataDevice::LDataDevicePrivate::sendDNDEnterEvent(LSurface *surface, Float64 x, Float64 y)
{

    if(seat->dndManager()->dragging() && seat->dndManager()->focus() != surface)
    {
        sendDNDLeaveEvent();

        if(seat->dndManager()->source())
        {

            wl_resource *dataOffer = wl_resource_create(client->client(),&wl_data_offer_interface,wl_resource_get_version(resource),0);
            LDataOffer *lDataOffer = new LDataOffer(dataOffer,dataDevice);
            lDataOffer->imp()->usedFor = LDataOffer::DND;
            lDataOffer->seat()->dndManager()->imp()->offer = lDataOffer;
            wl_resource_set_implementation(dataOffer, &dataOffer_implementation, lDataOffer, &Globals::DataOffer::resource_destroy);
            wl_data_device_send_data_offer(resource,dataOffer);

            for(const LDataSource::LSource &s : seat->dndManager()->source()->sources())
                wl_data_offer_send_offer(dataOffer,s.mimeType);

            wl_data_device_send_enter(resource,
                                     LWayland::nextSerial(),
                                     surface->resource(),
                                     wl_fixed_from_double(x/seat->compositor()->globalScale()),
                                     wl_fixed_from_double(y/seat->compositor()->globalScale()),
                                     dataOffer);


#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
            if(wl_resource_get_version(resource) >= 3 && wl_resource_get_version(seat->dndManager()->source()->resource()) >= 3)
                wl_data_offer_send_source_actions(dataOffer,seat->dndManager()->source()->dndActions());
#endif

            seat->dndManager()->imp()->focus = surface;


        }
        else
        {
            if(surface == seat->dndManager()->origin())
            {
                wl_data_device_send_enter(resource,
                                          LWayland::nextSerial(),
                                          surface->resource(),
                                          wl_fixed_from_double(x/seat->compositor()->globalScale()),
                                          wl_fixed_from_double(y/seat->compositor()->globalScale()),
                                          NULL);


                seat->dndManager()->imp()->focus = surface;
            }
        }


    }
}

void LDataDevice::LDataDevicePrivate::sendDNDMotionEvent(Float64 x, Float64 y)
{
    if(seat->dndManager()->dragging() && seat->dndManager()->focus())
    {
        if(seat->dndManager()->source() || (!seat->dndManager()->source() && seat->dndManager()->focus() == seat->dndManager()->origin()))
        {
            wl_data_device_send_motion(resource,
                                       LTime::ms(),
                                       wl_fixed_from_double(x/seat->compositor()->globalScale()),
                                       wl_fixed_from_double(y/seat->compositor()->globalScale()));
        }
    }
}

void LDataDevice::LDataDevicePrivate::sendDNDLeaveEvent()
{
    if(seat->dndManager()->dragging() && seat->dndManager()->focus())
        wl_data_device_send_leave(seat->dndManager()->focus()->client()->dataDevice()->resource());

    seat->dndManager()->imp()->matchedMimeType = false;
    seat->dndManager()->imp()->focus = nullptr;
}

LDataDevice::LDataDevicePrivate *LDataDevice::imp() const
{
    return m_imp;
}

