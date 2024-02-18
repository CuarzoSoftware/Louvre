#ifndef LTOUCHFRAMEEVENT_H
#define LTOUCHFRAMEEVENT_H

#include <LTouchEvent.h>
#include <LTime.h>

class Louvre::LTouchFrameEvent : public LTouchEvent
{
public:
    inline LTouchFrameEvent(UInt32 serial = LTime::nextSerial(),
                            UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LTouchEvent(LEvent::Subtype::Frame, serial, ms, us, device)
    {}

private:
    friend class LInputBackend;
    void notify();
};

#endif // LTOUCHFRAMEEVENT_H
