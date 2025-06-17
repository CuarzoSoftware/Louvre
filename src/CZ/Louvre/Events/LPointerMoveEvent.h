#ifndef LPOINTERMOVEEVENT_H
#define LPOINTERMOVEEVENT_H

#include <CZ/Louvre/Events/LPointerEvent.h>
#include <CZ/skia/core/SkPoint.h>
#include <CZ/Louvre/LTime.h>

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
    LPointerMoveEvent(SkPoint delta = SkPoint(0.f, 0.f), SkPoint deltaUnaccelerated = SkPoint(0.f, 0.f),
                             UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LPointerEvent(LEvent::Subtype::Move, serial, ms, us, device),
        m_delta(delta),
        m_deltaUnaccelerated(deltaUnaccelerated)
    {}

    /**
     * @brief Sets the movement delta of the pointer.
     */
    void setDelta(SkPoint delta) noexcept
    {
        m_delta = delta;
    }

    /**
     * @brief Sets the movement delta along the x-axis of the pointer.
     */
    void setDx(Float32 dx) noexcept
    {
        m_delta.fX = dx;
    }

    /**
     * @brief Sets the movement delta along the y-axis of the pointer.
     */
    void setDy(Float32 dy) noexcept
    {
        m_delta.fY = dy;
    }

    /**
     * @brief Gets the movement delta of the pointer.
     */
    SkPoint delta() const noexcept
    {
        return m_delta;
    }

    /**
     * @brief Sets the unaccelerated movement delta of the pointer.
     */
    void setDeltaUnaccelerated(SkPoint deltaUnaccelerated) noexcept
    {
        m_deltaUnaccelerated = deltaUnaccelerated;
    }

    /**
     * @brief Sets the unaccelerated movement delta along the x-axis of the pointer.
     */
    void setDxUnaccelerated(Float32 dx) noexcept
    {
        m_deltaUnaccelerated.fX = dx;
    }

    /**
     * @brief Sets the unaccelerated movement delta along the y-axis of the pointer.
     */
    void setDyUnaccelerated(Float32 dy) noexcept
    {
        m_deltaUnaccelerated.fY = dy;
    }

    /**
     * @brief Gets the unaccelerated movement delta of the pointer.
     */
    SkPoint deltaUnaccelerated() const noexcept
    {
        return m_deltaUnaccelerated;
    }

    /**
     * @brief The surface or view local position where the pointer is positioned in surface coordinates.
     */
    mutable SkPoint localPos;

protected:
    SkPoint m_delta;
    SkPoint m_deltaUnaccelerated;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERMOVEEVENT_H
