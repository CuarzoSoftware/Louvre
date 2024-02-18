#ifndef LKEYBOARDLEAVEEVENT_H
#define LKEYBOARDLEAVEEVENT_H

#include <LKeyboardEvent.h>
#include <LTime.h>

class Louvre::LKeyboardLeaveEvent : public LKeyboardEvent
{
public:
    inline LKeyboardLeaveEvent(UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LKeyboardEvent(LEvent::Subtype::Leave, serial, ms, us, device)
    {}
};

#endif // LKEYBOARDLEAVEEVENT_H
