#ifndef LPOINTERMOVEEVENT_H
#define LPOINTERMOVEEVENT_H

#include <LPointerEvent.h>
#include <LPoint.h>
#include <LTime.h>

class Louvre::LPointerMoveEvent : public LPointerEvent
{
public:
    inline LPointerMoveEvent(const LPointF &delta = LPointF(0.f, 0.f), const LPointF &deltaUnaccelerated = LPointF(0.f, 0.f),
                             UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::Move, serial, ms, us, device),
        m_delta(delta),
        m_deltaUnaccelerated(deltaUnaccelerated)
    {}

    inline void setDelta(const LPointF &delta)
    {
        m_delta = delta;
    }

    inline void setDx(Float32 dx)
    {
        m_delta.setX(dx);
    }

    inline void setDy(Float32 dy)
    {
        m_delta.setY(dy);
    }

    const LPointF &delta() const
    {
        return m_delta;
    }

    inline void setDeltaUnaccelerated(const LPointF &deltaUnaccelerated)
    {
        m_deltaUnaccelerated = deltaUnaccelerated;
    }

    inline void setDxUnaccelerated(Float32 dx)
    {
        m_deltaUnaccelerated.setX(dx);
    }

    inline void setDyUnaccelerated(Float32 dy)
    {
        m_deltaUnaccelerated.setY(dy);
    }

    const LPointF &deltaUnaccelerated() const
    {
        return m_deltaUnaccelerated;
    }

    mutable LPointF localPos;

protected:
    LPointF m_delta;
    LPointF m_deltaUnaccelerated;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERMOVEEVENT_H
