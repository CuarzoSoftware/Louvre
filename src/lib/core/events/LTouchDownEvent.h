#ifndef LTOUCHDOWNEVENT_H
#define LTOUCHDOWNEVENT_H

#include <LTouchEvent.h>
#include <LPoint.h>
#include <LTime.h>

/**
 * @brief Touch down event.
 */
class Louvre::LTouchDownEvent final : public LTouchEvent
{
public:
    /**
     * @brief Constructs an LTouchDownEvent object.
     *
     * @param id The unique identifier of the touch point.
     * @param pos The position of the touch point [0, 1].
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LTouchDownEvent(Int32 id = 0, const LPointF &pos = LPointF(0.f, 0.f), UInt32 serial = LTime::nextSerial(),
                           UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LTouchEvent(LEvent::Subtype::Down, serial, ms, us, device),
        m_id(id),
        m_pos(pos)
    {}

    /**
     * @brief Sets the position of the touch point.
     */
    void setPos(const LPointF &pos) noexcept
    {
        m_pos = pos;
    }

    /**
     * @brief Sets the position of the touch point.
     */
    void setPos(Float32 x, Float32 y) noexcept
    {
        m_pos.setX(x);
        m_pos.setY(y);
    }

    /**
     * @brief Sets the x-coordinate of the touch point position.
     */
    void setX(Float32 x) noexcept
    {
        m_pos.setX(x);
    }

    /**
     * @brief Sets the y-coordinate of the touch point position.
     */
    void setY(Float32 y) noexcept
    {
        m_pos.setY(y);
    }

    /**
     * @brief Gets the position of the touch point.
     *
     * @note The position is typically normalized to the range [0, 1] for both axes.
     */
    const LPointF &pos() const noexcept
    {
        return m_pos;
    }

    /**
     * @brief Sets the unique identifier of the touch point.
     */
    void setId(Int32 id) noexcept
    {
        m_id = id;
    }

    /**
     * @brief Gets the unique identifier of the touch point.
     */
    Int32 id() const noexcept
    {
        return m_id;
    }

    /**
     * @brief The surface or view local position where the touch point is positioned in surface coordinates.
     */
    mutable LPointF localPos;

protected:
    Int32 m_id;
    LPointF m_pos;
private:
    friend class LInputBackend;
    friend class LTouchEvent;
    void notify();
};

#endif // LTOUCHDOWNEVENT_H
