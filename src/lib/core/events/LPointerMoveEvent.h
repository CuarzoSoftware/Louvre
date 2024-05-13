#ifndef LPOINTERMOVEEVENT_H
#define LPOINTERMOVEEVENT_H

#include <LPointerEvent.h>
#include <LPoint.h>
#include <LTime.h>

/**
 * @brief Pointer movement event.
 */
class Louvre::LPointerMoveEvent final : public LPointerEvent
{
public:
    /**
     * @brief Constructs an LPointerMoveEvent object.
     *
     * @param delta The movement delta of the pointer.
     * @param deltaUnaccelerated The unaccelerated movement delta of the pointer.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LPointerMoveEvent(const LPointF &delta = LPointF(0.f, 0.f), const LPointF &deltaUnaccelerated = LPointF(0.f, 0.f),
                             UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LPointerEvent(LEvent::Subtype::Move, serial, ms, us, device),
        m_delta(delta),
        m_deltaUnaccelerated(deltaUnaccelerated)
    {}

    /**
     * @brief Sets the movement delta of the pointer.
     */
    void setDelta(const LPointF &delta) noexcept
    {
        m_delta = delta;
    }

    /**
     * @brief Sets the movement delta along the x-axis of the pointer.
     */
    void setDx(Float32 dx) noexcept
    {
        m_delta.setX(dx);
    }

    /**
     * @brief Sets the movement delta along the y-axis of the pointer.
     */
    void setDy(Float32 dy) noexcept
    {
        m_delta.setY(dy);
    }

    /**
     * @brief Gets the movement delta of the pointer.
     */
    const LPointF &delta() const noexcept
    {
        return m_delta;
    }

    /**
     * @brief Sets the unaccelerated movement delta of the pointer.
     */
    void setDeltaUnaccelerated(const LPointF &deltaUnaccelerated) noexcept
    {
        m_deltaUnaccelerated = deltaUnaccelerated;
    }

    /**
     * @brief Sets the unaccelerated movement delta along the x-axis of the pointer.
     */
    void setDxUnaccelerated(Float32 dx) noexcept
    {
        m_deltaUnaccelerated.setX(dx);
    }

    /**
     * @brief Sets the unaccelerated movement delta along the y-axis of the pointer.
     */
    void setDyUnaccelerated(Float32 dy) noexcept
    {
        m_deltaUnaccelerated.setY(dy);
    }

    /**
     * @brief Gets the unaccelerated movement delta of the pointer.
     */
    const LPointF &deltaUnaccelerated() const noexcept
    {
        return m_deltaUnaccelerated;
    }

    /**
     * @brief The surface or view local position where the pointer is positioned in surface coordinates.
     */
    mutable LPointF localPos;

protected:
    LPointF m_delta;
    LPointF m_deltaUnaccelerated;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERMOVEEVENT_H
