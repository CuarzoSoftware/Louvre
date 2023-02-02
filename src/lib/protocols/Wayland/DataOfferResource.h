#ifndef DATAOFFERRESOURCE_H
#define DATAOFFERRESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::DataOfferResource : public LResource
{
public:
    DataOfferResource(DataDeviceResource *dataDeviceResource, UInt32 id);
    ~DataOfferResource();

    void sendAction(UInt32 action);
    void sendSourceActions(UInt32 actions);
    void sendOffer(const char *mimeType);

    LDataOffer *dataOffer() const;
    DataDeviceResource *dataDeviceResource() const;

    LPRIVATE_IMP(DataOfferResource)
};

#endif // DATAOFFERRESOURCE_H
