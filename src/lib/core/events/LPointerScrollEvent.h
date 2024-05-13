#ifndef LPOINTERAXISEVENT_H
#define LPOINTERAXISEVENT_H

#include <LPointerEvent.h>
#include <LPointer.h>
#include <LTime.h>

/**
 * @brief Pointer scroll event.
 */
class Louvre::LPointerScrollEvent final : public LPointerEvent
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

    /**
     * @brief Constructs an LPointerScrollEvent object.
     *
     * @param axes The scroll axes values.
     * @param axes120 The scroll axes values for high-resolution scrolling.
     * @param source The source of the scroll event.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LPointerScrollEvent(const LPointF &axes = LPointF(0.f, 0.f), const LPointF &axes120 = LPointF(0.f, 0.f), Source source = Continuous,
                               UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LPointerEvent(LEvent::Subtype::Scroll, serial, ms, us, device),
        m_axes(axes),
        m_axes120(axes120),
        m_source(source)
    {}

    /**
     * @brief Sets the scroll axes values.
     */
    void setAxes(const LPointF &axes) noexcept
    {
        m_axes = axes;
    }

    /**
     * @brief Sets the scroll axes values.
     */
    void setAxes(Float32 x, Float32 y) noexcept
    {
        m_axes.setX(x);
        m_axes.setY(y);
    }

    /**
     * @brief Sets the scroll value along the x-axis.
     */
    void setX(Float32 x) noexcept
    {
        m_axes.setX(x);
    }

    /**
     * @brief Sets the scroll value along the y-axis.
     */
    void setY(Float32 y) noexcept
    {
        m_axes.setY(y);
    }

    /**
     * @brief Gets the scroll axes values.
     */
    const LPointF &axes() const noexcept
    {
        return m_axes;
    }

    /**
     * @brief Sets the high-resolution scroll axes values.
     */
    void setAxes120(const LPointF &axes) noexcept
    {
        m_axes120 = axes;
    }

    /**
     * @brief Sets the high-resolution scroll axes values.
     */
    void setAxes120(Float32 x, Float32 y) noexcept
    {
        m_axes120.setX(x);
        m_axes120.setY(y);
    }

    /**
     * @brief Sets the high-resolution scroll value along the x-axis.
     */
    void set120X(Float32 x) noexcept
    {
        m_axes120.setX(x);
    }

    /**
     * @brief Sets the high-resolution scroll value along the y-axis.
     */
    void set120Y(Float32 y) noexcept
    {
        m_axes120.setY(y);
    }

    /**
     * @brief Gets the high-resolution scroll axes values.
     *
     * A value that is a fraction of ±120 indicates a wheel movement less than one logical click, a caller should either scroll by
     * the respective fraction of the normal scroll distance or accumulate that value until a multiple of 120 is reached.
     *
     * @note Only for events with a @ref Wheel source.
     */
    const LPointF &axes120() const noexcept
    {
        return m_axes120;
    }

    /**
     * @brief Sets the source of the scroll event.
     */
    void setSource(Source source) noexcept
    {
        m_source = source;
    }

    /**
     * @brief Gets the source of the scroll event.
     */
    Source source() const noexcept
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
