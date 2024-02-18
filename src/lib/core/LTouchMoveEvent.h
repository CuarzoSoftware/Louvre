#ifndef LTOUCHMOVEEVENT_H
#define LTOUCHMOVEEVENT_H

#include <LTouchEvent.h>
#include <LPoint.h>
#include <LTime.h>

class Louvre::LTouchMoveEvent : public LTouchEvent
{
public:
    inline LTouchMoveEvent(Int32 id = 0, const LPointF &pos = LPointF(0.f, 0.f), UInt32 serial = LTime::nextSerial(),
                           UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LTouchEvent(LEvent::Subtype::Move, serial, ms, us, device),
        m_id(id),
        m_pos(pos)
    {}

    inline void setPos(const LPointF &pos)
    {
        m_pos = pos;
    }

    inline void setPos(Float32 x, Float32 y)
    {
        m_pos.setX(x);
        m_pos.setY(y);
    }

    inline void setX(Float32 x)
    {
        m_pos.setX(x);
    }

    inline void setY(Float32 y)
    {
        m_pos.setY(y);
    }

    inline const LPointF &pos() const
    {
        return m_pos;
    }

    inline void setId(Int32 id)
    {
        m_id = id;
    }

    inline Int32 id() const
    {
        return m_id;
    }

    mutable LPointF localPos;

protected:
    Int32 m_id;
    LPointF m_pos;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LTOUCHMOVEEVENT_H
