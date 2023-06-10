#ifndef LDATADEVICEPRIVATE_H
#define LDATADEVICEPRIVATE_H

#include <LDataDevice.h>

using namespace Louvre;

LPRIVATE_CLASS(LDataDevice)

    LClient *client = nullptr;

    // Drag & Drop events
    void sendDNDEnterEventS(LSurface *surface, Float24 x, Float24 y);
    void sendDNDMotionEventS(Float24 x, Float24 y);
    void sendDNDLeaveEvent();
};

#endif // LDATADEVICEPRIVATE_H
