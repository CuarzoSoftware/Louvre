#ifndef LDATADEVICEPRIVATE_H
#define LDATADEVICEPRIVATE_H

#include <LDataDevice.h>

using namespace Louvre;

LPRIVATE_CLASS(LDataDevice)

    LClient *client = nullptr;

    // Drag & Drop events
    void sendDNDEnterEvent(LSurface *surface, Float64 x, Float64 y);
    void sendDNDMotionEvent(Float64 x, Float64 y);
    void sendDNDLeaveEvent();
};

#endif // LDATADEVICEPRIVATE_H
