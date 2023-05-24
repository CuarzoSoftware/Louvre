#ifndef DATAOFFERRESOURCE_H
#define DATAOFFERRESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RDataOffer : public LResource
{
public:
    RDataOffer(RDataDevice *dataDeviceResource, UInt32 id);
    ~RDataOffer();

    void sendAction(UInt32 action);
    void sendSourceActions(UInt32 actions);
    void sendOffer(const char *mimeType);

    LDataOffer *dataOffer() const;
    RDataDevice *dataDeviceResource() const;

    LPRIVATE_IMP(RDataOffer)
};

#endif // DATAOFFERRESOURCE_H
