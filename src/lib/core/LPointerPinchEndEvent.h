#ifndef LPOINTERPINCHENDEVENT_H
#define LPOINTERPINCHENDEVENT_H

#include <LPointerEvent.h>
#include <LTime.h>

class Louvre::LPointerPinchEndEvent : public LPointerEvent
{
public:
    inline LPointerPinchEndEvent(UInt32 fingers = 0, bool cancelled = false,
                                 UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::PinchEnd, serial, ms, us, device),
        m_fingers(fingers),
        m_cancelled(cancelled)
    {}

    inline void setFingers(UInt32 fingers)
    {
        m_fingers = fingers;
    }

    inline UInt32 fingers() const
    {
        return m_fingers;
    }

    inline void setCancelled(bool cancelled)
    {
        m_cancelled = cancelled;
    }

    inline bool cancelled() const
    {
        return m_cancelled;
    }

protected:
    UInt32 m_fingers;
    bool m_cancelled;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERPINCHENDEVENT_H
