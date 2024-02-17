#ifndef LPOINTERAXISEVENT_H
#define LPOINTERAXISEVENT_H

#include <LPointerEvent.h>
#include <LPointer.h>
#include <LTime.h>

class Louvre::LPointerScrollEvent : public LPointerEvent
{
public:
    /**
     * @brief Source of a scroll event
     *
     * Possible sources of a scroll event.
     */
    enum Source : UInt32
    {
        /// Mouse wheel (discrete)
        Wheel = 0,

        /// Trackpad swipe (continuous)
        Finger = 1,

        /// Continuous movement (with unspecified source)
        Continuous = 2,

        /// Side movement of a mouse wheel (since 6)
        WheelTilt = 3
    };

    inline LPointerScrollEvent(const LPointF &axes = LPointF(0.f, 0.f), const LPointF &axes120 = LPointF(0.f, 0.f), Source source = Continuous,
                               UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::Scroll, serial, ms, us, device),
        m_axes(axes),
        m_axes120(axes120),
        m_source(source)
    {}

    inline void setAxes(const LPointF &axes)
    {
        m_axes = axes;
    }

    inline void setAxes(Float32 x, Float32 y)
    {
        m_axes.setX(x);
        m_axes.setY(y);
    }

    inline void setX(Float32 x)
    {
        m_axes.setX(x);
    }

    inline void setY(Float32 y)
    {
        m_axes.setY(y);
    }

    inline const LPointF &axes() const
    {
        return m_axes;
    }

    //

    inline void setAxes120(const LPointF &axes)
    {
        m_axes120 = axes;
    }

    inline void setAxes120(Float32 x, Float32 y)
    {
        m_axes120.setX(x);
        m_axes120.setY(y);
    }

    inline void set120X(Float32 x)
    {
        m_axes120.setX(x);
    }

    inline void set120Y(Float32 y)
    {
        m_axes120.setY(y);
    }

    inline const LPointF &axes120() const
    {
        return m_axes120;
    }

    inline void invertX()
    {
        m_axes.setX(-m_axes.x());
        m_axes120.setX(-m_axes120.x());
    }

    inline void invertY()
    {
        m_axes.setY(-m_axes.y());
        m_axes120.setY(-m_axes120.y());
    }

    //

    inline void setSource(Source source)
    {
        m_source = source;
    }

    inline Source source() const
    {
        return m_source;
    }

protected:
    LPointF m_axes;
    LPointF m_axes120;
    Source m_source;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERAXISEVENT_H
