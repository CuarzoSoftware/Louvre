#ifndef LPOINTERAXISEVENT_H
#define LPOINTERAXISEVENT_H

#include <LPointer.h>
#include <LPointerEvent.h>
#include <LTime.h>

/**
 * @brief Pointer scroll event.
 *
 * When handling mouse wheel events on the server, its recommended to choose
 * either
 * @ref Wheel or @ref WheelLegacy and avoid handling both due to their semantic
 * differences.
 *
 * However, all events should still be dispatched to clients through
 * LPointer::sendScrollEvent(). Louvre internally filters them appropriately
 * based on the `wl_pointer` version used by the client.
 */
class Louvre::LPointerScrollEvent final : public LPointerEvent {
 public:
  /**
   * @brief Source of a scroll event
   *
   * Possible sources of a scroll event.
   */
  enum Source : UInt32 {
    /// [Mouse wheel (120
    /// value)](https://wayland.freedesktop.org/libinput/doc/latest/api/group__event__pointer.html#ga31d3c555e912f021d3880d1cd7eb8a49)
    Wheel = 0,

    /// [Finger
    /// (continuous)](https://wayland.freedesktop.org/libinput/doc/latest/api/group__event__pointer.html#ga64ae33acadd4daf2144b906878f64882)
    Finger = 1,

    /// [Continuous movement (with unspecified
    /// source)](https://wayland.freedesktop.org/libinput/doc/latest/api/group__event__pointer.html#ga64ae33acadd4daf2144b906878f64882)
    Continuous = 2,

    /// [Side movement of a mouse
    /// wheel](https://wayland.freedesktop.org/libinput/doc/latest/api/group__device.html#gga76c012d8f6d7656fb795dc7bdf9d6551a60e7f4a41ceda06fe3eba2d512dc8ec9)
    WheelTilt = 3,

    /// [Legacy mouse wheel
    /// (discrete)](https://wayland.freedesktop.org/libinput/doc/latest/api/group__event__pointer.html#ga38d12fc6884c9943e261febdb2384b98)
    WheelLegacy = 1000
  };

  /**
   * @brief Relative directional information of the entity causing the axis
   * motion.
   */
  enum RelativeDirection : UInt8 {
    /// The scroll direction matches the physical movement (e.g., fingers move
    /// down, scroll moves down).
    Identical = static_cast<UInt8>(0),

    /// The scroll direction is reversed due to natural scrolling (e.g., fingers
    /// move down, scroll moves up).
    Inverted = static_cast<UInt8>(1)
  };

  /**
   * @brief Constructs an LPointerScrollEvent object.
   *
   * @param axes The scroll axes values (included in all sources).
   * @param axesDiscrete The scroll axes values for high-resolution scrolling
   * (@ref Wheel source) or th physical mouse wheel clicks (@ref WheelLegacy
   * source).
   * @param hasX Indicates whether the event includes a value for the X axis.
   * @param hasY Indicates whether the event includes a value for the Y axis.
   * @param relativeDirectionX The X axis relative direction.
   * @param relativeDirectionY The Y axis relative direction.
   * @param source The source of the scroll event.
   * @param serial The serial number of the event.
   * @param ms The millisecond timestamp of the event.
   * @param us The microsecond timestamp of the event.
   * @param device The input device that originated the event.
   */
  LPointerScrollEvent(const LPointF &axes = LPointF(0.f, 0.f),
                      const LPointF &axesDiscrete = LPoint(0, 0),
                      bool hasX = true, bool hasY = true,
                      RelativeDirection relativeDirectionX = Identical,
                      RelativeDirection relativeDirectionY = Identical,
                      Source source = Continuous,
                      UInt32 serial = LTime::nextSerial(),
                      UInt32 ms = LTime::ms(), UInt64 us = LTime::us(),
                      LInputDevice *device = nullptr) noexcept
      : LPointerEvent(LEvent::Subtype::Scroll, serial, ms, us, device),
        m_hasX(hasX),
        m_hasY(hasY),
        m_relativeDirectionX(relativeDirectionX),
        m_relativeDirectionY(relativeDirectionY),
        m_axes(axes),
        m_axesDiscrete(axesDiscrete),
        m_source(source) {}

  /**
   * @brief Indicates whether the event includes a value for the X axis.
   *
   * @note Applicable to all sources.
   */
  bool hasX() const noexcept { return m_hasX; }

  /**
   * @brief Sets whether the event includes a value for the X axis.
   *
   * @note Applicable to all sources.
   */
  void setHasX(bool hasX) noexcept { m_hasX = hasX; }

  /**
   * @brief Indicates whether the event includes a value for the Y axis.
   *
   * @note Applicable to all sources.
   */
  bool hasY() const noexcept { return m_hasY; }

  /**
   * @brief Sets whether the event includes a value for the Y axis.
   *
   * @note Applicable to all sources.
   */
  void setHasY(bool hasY) noexcept { m_hasY = hasY; }

  /**
   * @brief Sets the scroll axes values.
   *
   * @note Applicable to all sources.
   */
  void setAxes(const LPointF &axes) noexcept { m_axes = axes; }

  /**
   * @brief Sets the scroll axes values.
   *
   * @note Applicable to all sources.
   */
  void setAxes(Float32 x, Float32 y) noexcept {
    m_axes.setX(x);
    m_axes.setY(y);
  }

  /**
   * @brief Sets the scroll value along the x-axis.
   *
   * @note Applicable to all sources.
   */
  void setX(Float32 x) noexcept { m_axes.setX(x); }

  /**
   * @brief Sets the scroll value along the y-axis.
   *
   * @note Applicable to all sources.
   */
  void setY(Float32 y) noexcept { m_axes.setY(y); }

  /**
   * @brief Gets the scroll axes values.
   *
   * @note Applicable to all sources.
   */
  const LPointF &axes() const noexcept { return m_axes; }

  /**
   * @brief Sets the discrete scroll axes values.
   *
   * @see discreteAxes()
   */
  void setDiscreteAxes(const LPoint &axes) noexcept { m_axesDiscrete = axes; }

  /**
   * @brief Sets the discrete scroll axes values using individual x and y
   * components.
   *
   * @see discreteAxes()
   */
  void setDiscreteAxes(Int32 x, Int32 y) noexcept {
    m_axesDiscrete.setX(x);
    m_axesDiscrete.setY(y);
  }

  /**
   * @brief Sets the discrete scroll value along the x-axis.
   *
   * @see discreteAxes()
   */
  void setDiscreteX(Int32 x) noexcept { m_axesDiscrete.setX(x); }

  /**
   * @brief Sets the discrete scroll value along the y-axis.
   *
   * @see discreteAxes()
   */
  void setDiscreteY(Int32 y) noexcept { m_axesDiscrete.setY(y); }

  /**
   * @brief Retrieves the discrete scroll axes values.
   *
   * - If the source is @ref WheelLegacy, the values represent physical mouse
   * wheel clicks.
   *
   * - If the source is @ref Wheel, the property contains high-resolution scroll
   * axis values: A value that is a fraction of ±120 indicates a wheel movement
   * smaller than one logical click. The caller should either scroll by the
   * respective fraction of the normal scroll distance or accumulate the value
   * until it reaches a multiple of 120.
   *
   * Ignore this value for other source types.
   */
  const LPoint &discreteAxes() const noexcept { return m_axesDiscrete; }

  /**
   * @brief Gets the relative scroll direction along the X axis.
   */
  RelativeDirection relativeDirectionX() const noexcept {
    return m_relativeDirectionX;
  }

  /**
   * @brief Gets the relative scroll direction along the Y axis.
   */
  RelativeDirection relativeDirectionY() const noexcept {
    return m_relativeDirectionY;
  }

  /**
   * @brief Sets the relative scroll direction along the X axis.
   */
  void setRelativeDirectionX(RelativeDirection direction) noexcept {
    m_relativeDirectionX = direction;
  }

  /**
   * @brief Sets the relative scroll direction along the Y axis.
   */
  void setRelativeDirectionY(RelativeDirection direction) noexcept {
    m_relativeDirectionY = direction;
  }

  /**
   * @brief Sets the source of the scroll event.
   */
  void setSource(Source source) noexcept { m_source = source; }

  /**
   * @brief Gets the source of the scroll event.
   */
  Source source() const noexcept { return m_source; }

 protected:
  bool m_hasX;
  bool m_hasY;
  RelativeDirection m_relativeDirectionX;
  RelativeDirection m_relativeDirectionY;
  LPointF m_axes;
  LPoint m_axesDiscrete;
  Source m_source;

 private:
  friend class LInputBackend;
  void notify();
};

#endif  // LPOINTERAXISEVENT_H
