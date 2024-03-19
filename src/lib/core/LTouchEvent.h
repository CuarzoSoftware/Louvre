#ifndef LTOUCHEVENT_H
#define LTOUCHEVENT_H

#include <LInputEvent.h>

/// @cond OMIT
class Louvre::LTouchEvent : public LInputEvent
{
protected:
    inline LTouchEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, LInputDevice *device) :
        LInputEvent(Type::Touch, subtype, serial, ms, us, device)
    {}
};
/// @endcond

#endif // LTOUCHEVENT_H
