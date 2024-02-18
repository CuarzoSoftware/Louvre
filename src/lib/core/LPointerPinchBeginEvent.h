#ifndef LPOINTERPINCHBEGINEVENT_H
#define LPOINTERPINCHBEGINEVENT_H

#include <LPointerEvent.h>
#include <LTime.h>

class Louvre::LPointerPinchBeginEvent : public LPointerEvent
{
public:
    inline LPointerPinchBeginEvent(UInt32 fingers = 0,
                                   UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::PinchBegin, serial, ms, us, device),
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

#endif // LPOINTERPINCHBEGINEVENT_H
