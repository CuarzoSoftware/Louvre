#ifndef LPOINTERPINCHUPDATEEVENT_H
#define LPOINTERPINCHUPDATEEVENT_H

#include <LPointerEvent.h>
#include <LPoint.h>
#include <LTime.h>

class Louvre::LPointerPinchUpdateEvent : public LPointerEvent
{
public:
    inline LPointerPinchUpdateEvent(UInt32 fingers = 0, const LPointF &delta = LPointF(0.f, 0.f), const LPointF &deltaUnaccelerated = LPointF(0.f, 0.f),
                                    Float32 scale = 1.f, Float32 rotation = 0.f,
                                    UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::PinchUpdate, serial, ms, us, device),
        m_fingers(fingers),
        m_delta(delta),
        m_deltaUnaccelerated(deltaUnaccelerated),
        m_scale(scale),
        m_rotation(rotation)
    {}

    inline void setFingers(UInt32 fingers)
    {
        m_fingers = fingers;
    }

    inline UInt32 fingers() const
    {
        return m_fingers;
    }

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

    inline void setScale(Float32 scale)
    {
        m_scale = scale;
    }

    inline Float32 scale() const
    {
        return m_scale;
    }

    inline void setRotation(Float32 rotation)
    {
        m_rotation = rotation;
    }

    inline Float32 rotation() const
    {
        return m_rotation;
    }

protected:
    UInt32 m_fingers;
    LPointF m_delta;
    LPointF m_deltaUnaccelerated;
    Float32 m_scale;
    Float32 m_rotation;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERPINCHUPDATEEVENT_H
