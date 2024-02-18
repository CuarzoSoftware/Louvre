#ifndef LKEYBOARDEVENT_H
#define LKEYBOARDEVENT_H

#include <LInputEvent.h>

class Louvre::LKeyboardEvent : public LInputEvent
{
protected:
    inline LKeyboardEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, LInputDevice *device) :
        LInputEvent(Type::Keyboard, subtype, serial, ms, us, device)
    {}
};

#endif // LKEYBOARDEVENT_H
