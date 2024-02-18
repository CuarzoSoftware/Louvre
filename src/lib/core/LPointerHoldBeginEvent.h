#ifndef LPOINTERHOLDBEGINEVENT_H
#define LPOINTERHOLDBEGINEVENT_H

#include <LPointerEvent.h>
#include <LTime.h>

class Louvre::LPointerHoldBeginEvent : public LPointerEvent
{
public:
    inline LPointerHoldBeginEvent(UInt32 fingers = 0,
                                  UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::HoldBegin, serial, ms, us, device),
        m_fingers(fingers)
    {}

    inline void setFingers(UInt32 fingers)
    {
        m_fingers = fingers;
    }

    inline UInt32 fingers() const
    {
        return m_fingers;
    }

protected:
    UInt32 m_fingers;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERHOLDBEGINEVENT_H
