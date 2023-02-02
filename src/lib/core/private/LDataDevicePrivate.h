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

    LClient *client                                             = nullptr;

    // Drag & Drop events
    void sendDNDEnterEvent(LSurface *surface, Float64 x, Float64 y);
    void sendDNDMotionEvent(Float64 x, Float64 y);
    void sendDNDLeaveEvent();
};


#endif // LDATADEVICEPRIVATE_H
