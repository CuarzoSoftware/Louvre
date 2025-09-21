#ifndef LTOUCHPOINT_H
#define LTOUCHPOINT_H

#include <LObject.h>
#include <LTouchDownEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>
#include <LWeak.h>

/**
 * @brief Touch point within a touch device.
 *
 * An instance of this class is created using LTouch::createOrGetTouchPoint()
 * and is typically destroyed after calling LTouch::sendFrameEvent() when the
 * touch point is no longer pressed, or when invoking LTouch::sendCancelEvent().
 *
 * A touch point can be assigned to a single surface at a time or none at all.
 * This assignment is performed using the sendDownEvent() method.
 *
 * To gain a comprehensive understanding of touch events, please refer to the
 * documentation of LTouch.
 *
 * @note When sending a touch-down or move event to a surface, ensure that the
 * position is specified in local surface coordinates. Both LTouchDownEvent and
 * LTouchMoveEvent contain a mutable variable named `localPos`. Be sure to set
 * its value properly to the local surface coordinate before dispatching these
 * events.
 */
class Louvre::LTouchPoint final : public LObject {
 public:
  /**
   * @brief Gets the unique identifier of the touch point.
   *
   * Each touch point is assigned a unique identifier, ensuring that there are
   * no two touch points with the same ID.
   *
   * @return The unique identifier of the touch point.
   */
  Int32 id() const noexcept { return m_lastDownEvent.id(); }

  /**
   * @brief Check if the touch point is currently being pressed.
   *
   * This method indicates whether the touch point is in a pressed state.
   * Initially, all touch points are marked as pressed upon creation. The state
   * can change if either the sendDownEvent() or sendUpEvent() methods are
   * invoked.
   *
   * If LTouch::sendFrameEvent() is called and the touch point is no longer
   * pressed, it will be automatically destroyed.
   *
   * @return `true` if the touch point is currently pressed, `false` otherwise.
   */
  bool pressed() const noexcept { return m_pressed; }

  /**
   * @brief Gets the surface currently being touched by this touch point.
   *
   * The surface associated with this touch point is set using the
   * sendDownEvent() method. If no surface is assigned to this touch point, this
   * method returns `nullptr`.
   *
   * @return A pointer to the LSurface being touched by this touch point, or
   * `nullptr` if no surface is assigned.
   */
  LSurface *surface() const noexcept { return m_surface; }

  /**
   * @brief Gets the last touch-down event sent with sendDownEvent().
   */
  const LTouchDownEvent &lastDownEvent() const noexcept {
    return m_lastDownEvent;
  }

  /**
   * @brief Gets the last touch move event sent with sendMoveEvent().
   */
  const LTouchMoveEvent &lastMoveEvent() const noexcept {
    return m_lastMoveEvent;
  }

  /**
   * @brief Gets the last touch-up event sent with sendUpEvent().
   */
  const LTouchUpEvent &lastUpEvent() const noexcept { return m_lastUpEvent; }

  /**
   * @brief Gets the position of the touch point assigned by the last touch-down
   * or move event.
   *
   * The position is represented in a coordinate space ranging from 0 to 1 for
   * both the x and y axes.
   *
   * @return A constant reference to the position of the touch point.
   */
  const LPointF &pos() const noexcept { return m_pos; }

  /**
   * @brief Mark the touch point as pressed.
   *
   * If a surface is specified, the event is sent to that surface. If the
   * surface is different from the current surface, the previous surface
   * receives a touch-up event and a frame event. If the surface parameter is
   * `nullptr`, it unsets the current surface. If the current assigned surface
   * is destroyed, it is automatically unset.
   *
   * @note The LTouchDownEvent::localPos mutable variable must be set to
   * represent the position in local surface coordinates.
   *
   * @param event The touch-down event with information about the touch point
   * @param surface The surface to which the event should be sent. If `nullptr`,
   * unsets the surface.
   * @return `true` on success, `false` if the IDs don't match.
   */
  bool sendDownEvent(const LTouchDownEvent &event,
                     LSurface *surface = nullptr) noexcept;

  /**
   * @brief Notify the client about the movement of the touch point (if there is
   * a surface assigned).
   *
   * @note The LTouchMoveEvent::localPos mutable variable must be set to
   * represent the position in local surface coordinates.
   *
   * @param event The touch move event containing information about the touch
   * point movement.
   * @return `true` on success, `false` if the IDs don't match.
   */
  bool sendMoveEvent(const LTouchMoveEvent &event) noexcept;

  /**
   * @brief Notify the client about the touch point being released (if there is
   * a surface assigned).
   *
   * If LTouch::sendFrameEvent() is called and the touch point is no longer
   * pressed, it is automatically destroyed.
   *
   * @param event The touch-up event containing information about the touch
   * point release.
   * @return `true` on success, `false` if the IDs don't match.
   */
  bool sendUpEvent(const LTouchUpEvent &event) noexcept;

 private:
  friend class LTouch;
  LTouchPoint(const LTouchDownEvent &event) noexcept;
  ~LTouchPoint() noexcept { notifyDestruction(); };
  void sendTouchDownEvent(const LTouchDownEvent &event) noexcept;
  void sendTouchFrameEvent() noexcept;
  void sendTouchCancelEvent() noexcept;
  void resetSerials() noexcept;
  LWeak<LSurface> m_surface;
  LTouchDownEvent m_lastDownEvent;
  LTouchMoveEvent m_lastMoveEvent;
  LTouchUpEvent m_lastUpEvent;
  LPointF m_pos;
  bool m_pressed{true};
};

#endif  // LTOUCHPOINT_H
