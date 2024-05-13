#ifndef LTOUCHEVENT_H
#define LTOUCHEVENT_H

#include <LInputEvent.h>

/**
 * @brief Base class for touch events.
 *
 * All touch events share the same LEvent::Type::Touch type.
 */
class Louvre::LTouchEvent : public LInputEvent
{
protected:
    LTouchEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, LInputDevice *device) noexcept :
        LInputEvent(Type::Touch, subtype, serial, ms, us, device)
    {}
};

#endif // LTOUCHEVENT_H
