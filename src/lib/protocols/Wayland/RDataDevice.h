#ifndef RDATADEVICE_H
#define RDATADEVICE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RDataDevice : public LResource
{
public:
    RDataDevice(GDataDeviceManager *gDataDeviceManager, GSeat *gSeat, Int32 id);
    ~RDataDevice();

    struct LastEventSerials
    {
        UInt32 enter = 0;
    };

    GSeat *seatGlobal() const;
    LDataOffer *dataOffered() const;
    const LastEventSerials &serials() const;

    // Since 1
    bool dataOffer(RDataOffer *id);
    bool enter(UInt32 serial, RSurface *surface, Float24 x, Float24 y, RDataOffer *id);
    bool leave();
    bool motion(UInt32 time, Float24 x, Float24 y);
    bool drop();
    bool selection(RDataOffer *id);

    LPRIVATE_IMP_UNIQUE(RDataDevice)
};

#endif // RDATADEVICE_H
