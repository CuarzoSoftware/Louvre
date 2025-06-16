#ifndef LPOINTERPINCHUPDATEEVENT_H
#define LPOINTERPINCHUPDATEEVENT_H

#include <LPointerEvent.h>
#include <CZ/skia/core/SkPoint.h>
#include <LTime.h>

/**
 * @brief Pointer pinch update gesture event.
 */
class Louvre::LPointerPinchUpdateEvent final : public LPointerEvent
{
public:
    /**
     * @brief Constructs an LPointerPinchUpdateEvent object.
     *
     * @param fingers The number of fingers involved in the pinch gesture.
     * @param delta The movement delta of the pinch gesture.
     * @param deltaUnaccelerated The unaccelerated movement delta of the pinch gesture.
     * @param scale The scale factor of the pinch gesture.
     * @param rotation The rotation angle of the pinch gesture.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LPointerPinchUpdateEvent(UInt32 fingers = 0, const SkPoint &delta = SkPoint(0.f, 0.f), const SkPoint &deltaUnaccelerated = SkPoint(0.f, 0.f),
                                    Float32 scale = 1.f, Float32 rotation = 0.f,
                                    UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LPointerEvent(LEvent::Subtype::PinchUpdate, serial, ms, us, device),
        m_fingers(fingers),
        m_delta(delta),
        m_deltaUnaccelerated(deltaUnaccelerated),
        m_scale(scale),
        m_rotation(rotation)
    {}

    /**
     * @brief Sets the number of fingers involved in the pinch gesture.
     */
    void setFingers(UInt32 fingers) noexcept
    {
        m_fingers = fingers;
    }

    /**
     * @brief Gets the number of fingers involved in the pinch gesture.
     */
    UInt32 fingers() const noexcept
    {
        return m_fingers;
    }

    /**
     * @brief Sets the movement delta of the pinch gesture.
     */
    void setDelta(const SkPoint &delta) noexcept
    {
        m_delta = delta;
    }

    /**
     * @brief Sets the movement delta along the x-axis of the pinch gesture.
     */
    void setDx(Float32 dx) noexcept
    {
        m_delta.fX = dx;
    }

    /**
     * @brief Sets the movement delta along the y-axis of the pinch gesture.
     */
    void setDy(Float32 dy) noexcept
    {
        m_delta.fY = dy;
    }

    /**
     * @brief Gets the movement delta of the pinch gesture.
     */
    SkPoint delta() const noexcept
    {
        return m_delta;
    }

    /**
     * @brief Sets the unaccelerated movement delta of the pinch gesture.
     */
    void setDeltaUnaccelerated(const SkPoint &deltaUnaccelerated) noexcept
    {
        m_deltaUnaccelerated = deltaUnaccelerated;
    }

    /**
     * @brief Sets the unaccelerated movement delta along the x-axis of the pinch gesture.
     */
    void setDxUnaccelerated(Float32 dx) noexcept
    {
        m_deltaUnaccelerated.fX = dx;
    }

    /**
     * @brief Sets the unaccelerated movement delta along the y-axis of the pinch gesture.
     */
    void setDyUnaccelerated(Float32 dy) noexcept
    {
        m_deltaUnaccelerated.fY = dy;
    }

    /**
     * @brief Gets the unaccelerated movement delta of the pinch gesture.
     */
    SkPoint deltaUnaccelerated() const noexcept
    {
        return m_deltaUnaccelerated;
    }

    /**
     * @brief Sets the scale factor of the pinch gesture.
     */
    void setScale(Float32 scale) noexcept
    {
        m_scale = scale;
    }

    /**
     * @brief Gets the scale factor of the pinch gesture.
     *
     * The scale begins at 1.0, and if e.g. the fingers moved together by 50% then the scale will become 0.5,
     * if they move twice as far apart as initially the scale becomes 2.0, etc.
     */
    Float32 scale() const noexcept
    {
        return m_scale;
    }

    /**
     * @brief Sets the rotation angle of the pinch gesture.
     */
    void setRotation(Float32 rotation) noexcept
    {
        m_rotation = rotation;
    }

    /**
     * @brief Gets the rotation angle of the pinch gesture.
     *
     * The angle delta is defined as the change in angle of the line formed by the 2 fingers of a pinch gesture.
     * Clockwise rotation is represented by a positive delta, counter-clockwise by a negative delta.
     * If e.g. the fingers are on the 12 and 6 location of a clock face plate and they move to the 1 resp. 7 location in a single event then the angle delta is 30 degrees.
     */
    Float32 rotation() const noexcept
    {
        return m_rotation;
    }

protected:
    UInt32 m_fingers;
    SkPoint m_delta;
    SkPoint m_deltaUnaccelerated;
    Float32 m_scale;
    Float32 m_rotation;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERPINCHUPDATEEVENT_H
