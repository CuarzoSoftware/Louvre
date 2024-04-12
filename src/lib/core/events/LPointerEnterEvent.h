#ifndef LPOINTERENTEREVENT_H
#define LPOINTERENTEREVENT_H

#include <events/LPointerEvent.h>
#include <LPoint.h>
#include <LTime.h>

/**
 * @brief Event generated when a surface or view gains pointer focus.
 */
class Louvre::LPointerEnterEvent final : public LPointerEvent
{
public:
    /**
     * @brief Constructs an LPointerEnterEvent object.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LPointerEnterEvent(UInt32 serial = LTime::nextSerial(),
                              UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LPointerEvent(LEvent::Subtype::Enter, serial, ms, us, device)
    {}

    /**
     * @brief The surface or view local position where the pointer entered in surface coordinates.
     */
    mutable LPointF localPos;
};

#endif // LPOINTERENTEREVENT_H
