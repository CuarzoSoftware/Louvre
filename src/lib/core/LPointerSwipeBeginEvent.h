#ifndef LPOINTERSWIPEBEGINEVENT_H
#define LPOINTERSWIPEBEGINEVENT_H

#include <LPointerEvent.h>
#include <LTime.h>

class Louvre::LPointerSwipeBeginEvent : public LPointerEvent
{
public:
    inline LPointerSwipeBeginEvent(UInt32 fingers = 0,
                                   UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::SwipeBegin, serial, ms, us, device),
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


#endif // LPOINTERSWIPEBEGINEVENT_H
