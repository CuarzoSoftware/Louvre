#ifndef LPOINTEREVENT_H
#define LPOINTEREVENT_H

#include <LInputEvent.h>

class Louvre::LPointerEvent : public LInputEvent
{
protected:
    inline LPointerEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, LInputDevice *device) :
        LInputEvent(Type::Pointer, subtype, serial, ms, us, device)
    {}
};

#endif // LPOINTEREVENT_H
