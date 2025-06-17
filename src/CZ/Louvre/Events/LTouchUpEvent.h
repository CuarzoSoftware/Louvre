#ifndef LTOUCHUPEVENT_H
#define LTOUCHUPEVENT_H

#include <CZ/Louvre/Events/LTouchEvent.h>
#include <CZ/Louvre/LTime.h>

/**
 * @brief Touch up event.
 */
class Louvre::LTouchUpEvent final : public LTouchEvent
{
public:
    /**
     * @brief Constructs an LTouchUpEvent object.
     *
     * @param id The ID of the touch point.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LTouchUpEvent(Int32 id = 0, UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LTouchEvent(LEvent::Subtype::Up, serial, ms, us, device),
        m_id(id)
    {}

    /**
     * @brief Sets the ID of the touch point.
     */
    void setId(Int32 id) noexcept
    {
        m_id = id;
    }

    /**
     * @brief Gets the ID of the touch point.
     */
    Int32 id() const noexcept
    {
        return m_id;
    }

protected:
    Int32 m_id;
private:
    friend class LInputBackend;
    friend class LTouchEvent;
    void notify();
};

#endif // LTOUCHUPEVENT_H
