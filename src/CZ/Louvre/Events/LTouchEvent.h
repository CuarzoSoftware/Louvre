#ifndef LTOUCHEVENT_H
#define LTOUCHEVENT_H

#include <CZ/Louvre/Events/LInputEvent.h>

/**
 * @brief Base class for touch events.
 *
 * All touch events share the same LEvent::Type::Touch type.
 */
class Louvre::LTouchEvent : public LInputEvent
{
public:
    /**
     * @brief Gets the unique identifier of the touch point.
     *
     * @note If the subtype is @ref LEvent::Subtype::Frame or @ref LEvent::Subtype::Cancel -1 is returned.
     */
    Int32 id() const noexcept;

protected:
    LTouchEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, LInputDevice *device) noexcept :
        LInputEvent(Type::Touch, subtype, serial, ms, us, device)
    {}
};

#endif // LTOUCHEVENT_H
