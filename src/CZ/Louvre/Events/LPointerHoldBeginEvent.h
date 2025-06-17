#ifndef LPOINTERHOLDBEGINEVENT_H
#define LPOINTERHOLDBEGINEVENT_H

#include <CZ/Louvre/Events/LPointerEvent.h>
#include <CZ/Louvre/LTime.h>

/**
 * @brief Pointer hold begin gesture event.
 */
class Louvre::LPointerHoldBeginEvent final : public LPointerEvent
{
public:
    /**
     * @brief Constructs an LPointerHoldBeginEvent object.
     *
     * @param fingers The number of fingers involved in the hold gesture.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LPointerHoldBeginEvent(UInt32 fingers = 0, UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(),
                           UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LPointerEvent(LEvent::Subtype::HoldBegin, serial, ms, us, device),
        m_fingers(fingers)
    {}

    /**
     * @brief Sets the number of fingers involved in the hold gesture.
     */
    void setFingers(UInt32 fingers) noexcept
    {
        m_fingers = fingers;
    }

    /**
     * @brief Gets the number of fingers involved in the hold gesture.
     */
    UInt32 fingers() const noexcept
    {
        return m_fingers;
    }

protected:
    UInt32 m_fingers;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERHOLDBEGINEVENT_H
