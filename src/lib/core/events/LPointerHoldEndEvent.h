#ifndef LPOINTERHOLDENDEVENT_H
#define LPOINTERHOLDENDEVENT_H

#include <events/LPointerEvent.h>
#include <LTime.h>

/**
 * @brief Pointer hold end gesture event.
 */
class Louvre::LPointerHoldEndEvent final : public LPointerEvent
{
public:
    /**
     * @brief Constructs an LPointerHoldEndEvent object.
     *
     * @param fingers The number of fingers involved in the hold gesture.
     * @param cancelled Indicates whether the gesture was cancelled.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LPointerHoldEndEvent(UInt32 fingers = 0, bool cancelled = false, UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(),
                         UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LPointerEvent(LEvent::Subtype::HoldEnd, serial, ms, us, device),
        m_fingers(fingers),
        m_cancelled(cancelled)
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

    /**
     * @brief Sets whether the gesture was cancelled.
     */
    void setCancelled(bool cancelled) noexcept
    {
        m_cancelled = cancelled;
    }

    /**
     * @brief Indicates whether the gesture was cancelled.
     */
    bool cancelled() const noexcept
    {
        return m_cancelled;
    }

    /// @cond OMIT
protected:
    UInt32 m_fingers;
    bool m_cancelled;
private:
    friend class LInputBackend;
    void notify();
    /// @endcond
};

#endif // LPOINTERHOLDENDEVENT_H
