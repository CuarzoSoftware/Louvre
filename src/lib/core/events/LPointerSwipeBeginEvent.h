#ifndef LPOINTERSWIPEBEGINEVENT_H
#define LPOINTERSWIPEBEGINEVENT_H

#include <LPointerEvent.h>
#include <LTime.h>

/**
 * @brief Pointer swipe begin gesture event.
 */
class Louvre::LPointerSwipeBeginEvent final : public LPointerEvent {
 public:
  /**
   * @brief Constructs an LPointerSwipeBeginEvent object.
   *
   * @param fingers The number of fingers involved in the swipe gesture.
   * @param serial The serial number of the event.
   * @param ms The millisecond timestamp of the event.
   * @param us The microsecond timestamp of the event.
   * @param device The input device that originated the event.
   */
  LPointerSwipeBeginEvent(UInt32 fingers = 0,
                          UInt32 serial = LTime::nextSerial(),
                          UInt32 ms = LTime::ms(), UInt64 us = LTime::us(),
                          LInputDevice *device = nullptr) noexcept
      : LPointerEvent(LEvent::Subtype::SwipeBegin, serial, ms, us, device),
        m_fingers(fingers) {}

  /**
   * @brief Sets the number of fingers involved in the swipe gesture.
   */
  void setFingers(UInt32 fingers) noexcept { m_fingers = fingers; }

  /**
   * @brief Gets the number of fingers involved in the swipe gesture.
   */
  UInt32 fingers() const noexcept { return m_fingers; }

 protected:
  UInt32 m_fingers;

 private:
  friend class LInputBackend;
  void notify();
};

#endif  // LPOINTERSWIPEBEGINEVENT_H
