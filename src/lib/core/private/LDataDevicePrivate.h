#ifndef LDATADEVICEPRIVATE_H
#define LDATADEVICEPRIVATE_H

#include <LDataDevice.h>

class Louvre::LDataDevice::LDataDevicePrivate
{
public:
    LDataDevicePrivate()                                        = default;
    ~LDataDevicePrivate()                                       = default;

    LDataDevicePrivate(const LDataDevicePrivate&)               = delete;
    LDataDevicePrivate &operator=(const LDataDevicePrivate&)    = delete;

    LDataDevice *dataDevice                                     = nullptr;
    LClient *client                                             = nullptr;
    LSeat *seat                                                 = nullptr;
    wl_resource *resource                                       = nullptr;
    LDataOffer *dataOffered                                     = nullptr;
    UInt32 lastDataOfferId                                      = 0;

    // Drag & Drop events
    void sendDNDEnterEvent(LSurface *surface, Float64 x, Float64 y);
    void sendDNDMotionEvent(Float64 x, Float64 y);
    void sendDNDLeaveEvent();
};


#endif // LDATADEVICEPRIVATE_H
