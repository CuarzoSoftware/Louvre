#ifndef RDATAOFFER_H
#define RDATAOFFER_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RDataOffer : public LResource
{
public:
    RDataOffer(RDataDevice *rDataDevice, UInt32 id);
    ~RDataOffer();

    LDataOffer *dataOffer() const;
    RDataDevice *dataDeviceResource() const;

    // Since 1
    bool offer(const char *mimeType);

    // Since 3
    bool sourceActions(UInt32 sourceActions);
    bool action(UInt32 dndAction);

    LPRIVATE_IMP(RDataOffer)
};

#endif // RDATAOFFER_H
