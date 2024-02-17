#ifndef LPOINTERLEAVEEVENT_H
#define LPOINTERLEAVEEVENT_H

#include <LPointerEvent.h>
#include <LPoint.h>
#include <LTime.h>

class Louvre::LPointerLeaveEvent : public LPointerEvent
{
public:
    inline LPointerLeaveEvent(UInt32 serial = LTime::nextSerial(),
                              UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::Leave, serial, ms, us, device)
    {}
};
#endif // LPOINTERLEAVEEVENT_H
