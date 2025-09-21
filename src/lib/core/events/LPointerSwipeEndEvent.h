#ifndef LPOINTERSWIPEENDEVENT_H
#define LPOINTERSWIPEENDEVENT_H

#include <LPointerEvent.h>
#include <LTime.h>

/**
 * @brief Pointer swipe end gesture event.
 */
class Louvre::LPointerSwipeEndEvent final : public LPointerEvent {
 public:
  /**
   * @brief Constructs an LPointerSwipeEndEvent object.
   *
   * @param fingers The number of fingers involved in the swipe gesture.
   * @param cancelled Indicates whether the gesture was cancelled.
   * @param serial The serial number of the event.
   * @param ms The millisecond timestamp of the event.
   * @param us The microsecond timestamp of the event.
   * @param device The input device that originated the event.
   */
  LPointerSwipeEndEvent(UInt32 fingers = 0, bool cancelled = false,
                        UInt32 serial = LTime::nextSerial(),
                        UInt32 ms = LTime::ms(), UInt64 us = LTime::us(),
                        LInputDevice *device = nullptr) noexcept
      : LPointerEvent(LEvent::Subtype::SwipeEnd, serial, ms, us, device),
        m_fingers(fingers),
        m_cancelled(cancelled) {}

  /**
   * @brief Sets the number of fingers involved in the swipe gesture.
   */
  void setFingers(UInt32 fingers) noexcept { m_fingers = fingers; }

  /**
   * @brief Gets the number of fingers involved in the swipe gesture.
   */
  UInt32 fingers() const noexcept { return m_fingers; }

  /**
   * @brief Sets whether the gesture was cancelled.
   */
  void setCancelled(bool cancelled) noexcept { m_cancelled = cancelled; }

  /**
   * @brief Gets whether the gesture was cancelled.
   */
  bool cancelled() const noexcept { return m_cancelled; }

 protected:
  UInt32 m_fingers;
  bool m_cancelled;

 private:
  friend class LInputBackend;
  void notify();
};

#endif  // LPOINTERSWIPEENDEVENT_H
