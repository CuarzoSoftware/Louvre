#ifndef LPOINTERSWIPEUPDATEEVENT_H
#define LPOINTERSWIPEUPDATEEVENT_H

#include <LPointerEvent.h>
#include <LPoint.h>
#include <LTime.h>

/**
 * @brief Pointer swipe update gesture event.
 */
class Louvre::LPointerSwipeUpdateEvent final : public LPointerEvent
{
public:
    /**
     * @brief Constructs an LPointerSwipeUpdateEvent object.
     *
     * @param fingers The number of fingers involved in the swipe gesture.
     * @param delta The change in position of the swipe gesture.
     * @param deltaUnaccelerated The unaccelerated change in position of the swipe gesture.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LPointerSwipeUpdateEvent(UInt32 fingers = 0, const LPointF &delta = LPointF(0.f, 0.f), const LPointF &deltaUnaccelerated = LPointF(0.f, 0.f),
                                    UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LPointerEvent(LEvent::Subtype::SwipeUpdate, serial, ms, us, device),
        m_fingers(fingers),
        m_delta(delta),
        m_deltaUnaccelerated(deltaUnaccelerated)
    {}

    /**
     * @brief Sets the number of fingers involved in the swipe gesture.
     */
    void setFingers(UInt32 fingers) noexcept
    {
        m_fingers = fingers;
    }

    /**
     * @brief Gets the number of fingers involved in the swipe gesture.
     */
    UInt32 fingers() const noexcept
    {
        return m_fingers;
    }

    /**
     * @brief Sets the change in position of the swipe gesture.
     */
    void setDelta(const LPointF &delta) noexcept
    {
        m_delta = delta;
    }

    /**
     * @brief Sets the change in position of the swipe gesture along the X-axis.
     */
    void setDx(Float32 dx) noexcept
    {
        m_delta.setX(dx);
    }

    /**
     * @brief Sets the change in position of the swipe gesture along the Y-axis.
     */
    void setDy(Float32 dy) noexcept
    {
        m_delta.setY(dy);
    }

    /**
     * @brief Gets the change in position of the swipe gesture.
     */
    const LPointF &delta() const noexcept
    {
        return m_delta;
    }

    /**
     * @brief Sets the unaccelerated change in position of the swipe gesture.
     */
    void setDeltaUnaccelerated(const LPointF &deltaUnaccelerated) noexcept
    {
        m_deltaUnaccelerated = deltaUnaccelerated;
    }

    /**
     * @brief Sets the unaccelerated change in position of the swipe gesture along the X-axis.
     */
    void setDxUnaccelerated(Float32 dx) noexcept
    {
        m_deltaUnaccelerated.setX(dx);
    }

    /**
     * @brief Sets the unaccelerated change in position of the swipe gesture along the Y-axis.
     */
    void setDyUnaccelerated(Float32 dy) noexcept
    {
        m_deltaUnaccelerated.setY(dy);
    }

    /**
     * @brief Gets the unaccelerated change in position of the swipe gesture.
     */
    const LPointF &deltaUnaccelerated() const noexcept
    {
        return m_deltaUnaccelerated;
    }

    /// @cond OMIT
protected:
    UInt32 m_fingers;
    LPointF m_delta;
    LPointF m_deltaUnaccelerated;
private:
    friend class LInputBackend;
    void notify();
    /// @endcond
};

#endif // LPOINTERSWIPEUPDATEEVENT_H
