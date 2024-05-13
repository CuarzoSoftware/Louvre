#ifndef LPOINTEREVENT_H
#define LPOINTEREVENT_H

#include <LInputEvent.h>

/**
 * @brief Base class for pointer events.
 *
 * All pointer events share the same LEvent::Type::Pointer type.
 */
class Louvre::LPointerEvent : public LInputEvent
{
protected:
    LPointerEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, LInputDevice *device) :
        LInputEvent(Type::Pointer, subtype, serial, ms, us, device)
    {}
};

#endif // LPOINTEREVENT_H
