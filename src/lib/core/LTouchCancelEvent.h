#ifndef LTOUCHCANCELEVENT_H
#define LTOUCHCANCELEVENT_H

#include <LTouchEvent.h>
#include <LTime.h>

class Louvre::LTouchCancelEvent : public LTouchEvent
{
public:
    inline LTouchCancelEvent(UInt32 serial = LTime::nextSerial(),
                             UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LTouchEvent(LEvent::Subtype::Cancel, serial, ms, us, device)
    {}

private:
    friend class LInputBackend;
    void notify();
};

#endif // LTOUCHCANCELEVENT_H
