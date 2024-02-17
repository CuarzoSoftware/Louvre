#ifndef LPOINTERENTEREVENT_H
#define LPOINTERENTEREVENT_H

#include <LPointerEvent.h>
#include <LPoint.h>
#include <LTime.h>

class Louvre::LPointerEnterEvent : public LPointerEvent
{
public:
    inline LPointerEnterEvent(UInt32 serial = LTime::nextSerial(),
                              UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::Enter, serial, ms, us, device)
    {}
    mutable LPointF localPos;
};

#endif // LPOINTERENTEREVENT_H
