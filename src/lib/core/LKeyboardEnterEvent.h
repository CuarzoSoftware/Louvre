#ifndef LKEYBOARDENTEREVENT_H
#define LKEYBOARDENTEREVENT_H

#include <LKeyboardEvent.h>
#include <LTime.h>

class Louvre::LKeyboardEnterEvent : public LKeyboardEvent
{
public:
    inline LKeyboardEnterEvent(UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LKeyboardEvent(LEvent::Subtype::Enter, serial, ms, us, device)
    {}
};

#endif // LKEYBOARDENTEREVENT_H
