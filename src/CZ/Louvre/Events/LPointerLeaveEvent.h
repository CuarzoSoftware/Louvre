#ifndef LPOINTERLEAVEEVENT_H
#define LPOINTERLEAVEEVENT_H

#include <CZ/Louvre/Events/LPointerEvent.h>
#include <CZ/skia/core/SkPoint.h>
#include <CZ/Louvre/LTime.h>

/**
 * @brief Event generated when a surface or view loses pointer focus.
 */
class Louvre::LPointerLeaveEvent final : public LPointerEvent
{
public:
    /**
     * @brief Constructs an LPointerLeaveEvent object.
     *
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LPointerLeaveEvent(UInt32 serial = LTime::nextSerial(),
                              UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LPointerEvent(LEvent::Subtype::Leave, serial, ms, us, device)
    {}
};
#endif // LPOINTERLEAVEEVENT_H
