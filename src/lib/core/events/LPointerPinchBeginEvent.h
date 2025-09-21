#ifndef LPOINTERPINCHBEGINEVENT_H
#define LPOINTERPINCHBEGINEVENT_H

#include <LPointerEvent.h>
#include <LTime.h>

/**
 * @brief Pointer pinch begin gesture event.
 */
class Louvre::LPointerPinchBeginEvent final : public LPointerEvent {
 public:
  /**
   * @brief Constructs an LPointerPinchBeginEvent object.
   *
   * @param fingers The number of fingers involved in the pinch gesture.
   * @param serial The serial number of the event.
   * @param ms The millisecond timestamp of the event.
   * @param us The microsecond timestamp of the event.
   * @param device The input device that originated the event.
   */
  LPointerPinchBeginEvent(UInt32 fingers = 0,
                          UInt32 serial = LTime::nextSerial(),
                          UInt32 ms = LTime::ms(), UInt64 us = LTime::us(),
                          LInputDevice *device = nullptr) noexcept
      : LPointerEvent(LEvent::Subtype::PinchBegin, serial, ms, us, device),
        m_fingers(fingers) {}

  /**
   * @brief Sets the number of fingers involved in the pinch gesture.
   */
  void setFingers(UInt32 fingers) noexcept { m_fingers = fingers; }

  /**
   * @brief Gets the number of fingers involved in the pinch gesture.
   */
  UInt32 fingers() const noexcept { return m_fingers; }

 protected:
  UInt32 m_fingers;

 private:
  friend class LInputBackend;
  void notify();
};

#endif  // LPOINTERPINCHBEGINEVENT_H
