#ifndef LTOUCHUPEVENT_H
#define LTOUCHUPEVENT_H

#include <LTouchEvent.h>
#include <LTime.h>

class Louvre::LTouchUpEvent : public LTouchEvent
{
public:
    inline LTouchUpEvent(Int32 id = 0, UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LTouchEvent(LEvent::Subtype::Up, serial, ms, us, device),
        m_id(id)
    {}

    inline void setId(Int32 id)
    {
        m_id = id;
    }

    inline Int32 id() const
    {
        return m_id;
    }

protected:
    Int32 m_id;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LTOUCHUPEVENT_H
