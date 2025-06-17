#ifndef LTOUCHFRAMEEVENT_H
#define LTOUCHFRAMEEVENT_H

#include <CZ/Louvre/Events/LTouchEvent.h>
#include <CZ/Louvre/LTime.h>

/**
 * @brief Represents a touch frame event.
 *
 * This event marks a set of touch events that belong logically together.
 */
class Louvre::LTouchFrameEvent final : public LTouchEvent
{
public:
    /**
     * @brief Constructs an LTouchFrameEvent object.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LTouchFrameEvent(UInt32 serial = LTime::nextSerial(),
                            UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LTouchEvent(LEvent::Subtype::Frame, serial, ms, us, device)
    {}

private:
    friend class LInputBackend;
    void notify();
};

#endif // LTOUCHFRAMEEVENT_H
