#ifndef LKEYBOARDEVENT_H
#define LKEYBOARDEVENT_H

#include <CZ/Louvre/Events/LInputEvent.h>

/**
 * @brief Base class for keyboard events.
 *
 * All keyboard events share the same LEvent::Type::Keyboard type.
 */
class Louvre::LKeyboardEvent : public LInputEvent
{
protected:
    LKeyboardEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, LInputDevice *device) noexcept :
        LInputEvent(Type::Keyboard, subtype, serial, ms, us, device)
    {}
};

#endif // LKEYBOARDEVENT_H
