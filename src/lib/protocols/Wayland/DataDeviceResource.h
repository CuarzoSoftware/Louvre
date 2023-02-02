#ifndef DATADEVICERESOURCE_H
#define DATADEVICERESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::DataDeviceResource : public LResource
{
public:
    DataDeviceResource(DataDeviceManagerGlobal *dataDeviceManagerGlobal, SeatGlobal *seatGlobal, Int32 id);
    ~DataDeviceResource();

    void sendEnter(LSurface *surface, Float64 x, Float64 y, DataOfferResource *dataOfferResource);
    void sendLeave();
    void sendMotion(Float64 x, Float64 y);
    void sendDrop();
    void sendDataOffer(DataOfferResource *dataOfferResource);
    void sendSelection(DataOfferResource *dataOfferResource);

    SeatGlobal *seatGlobal() const;
    LDataOffer *dataOffered() const;

    LPRIVATE_IMP(DataDeviceResource)
};

#endif // DATADEVICERESOURCE_H
