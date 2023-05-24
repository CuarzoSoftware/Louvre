#ifndef DATADEVICERESOURCE_H
#define DATADEVICERESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RDataDevice : public LResource
{
public:
    RDataDevice(GDataDeviceManager *dataDeviceManagerGlobal, GSeat *seatGlobal, Int32 id);
    ~RDataDevice();

    void sendEnter(LSurface *surface, Float64 x, Float64 y, RDataOffer *dataOfferResource);
    void sendLeave();
    void sendMotion(Float64 x, Float64 y);
    void sendDrop();
    void sendDataOffer(RDataOffer *dataOfferResource);
    void sendSelection(RDataOffer *dataOfferResource);

    GSeat *seatGlobal() const;
    LDataOffer *dataOffered() const;

    LPRIVATE_IMP(RDataDevice)
};

#endif // DATADEVICERESOURCE_H
