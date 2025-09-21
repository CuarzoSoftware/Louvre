#ifndef LTOUCHCANCELEVENT_H
#define LTOUCHCANCELEVENT_H

#include <LTime.h>
#include <LTouchEvent.h>

/**
 * @brief Touch cancel event.
 */
class Louvre::LTouchCancelEvent final : public LTouchEvent {
 public:
  /**
   * @brief Constructs an LTouchCancelEvent object.
   *
   * @param serial The serial number of the event.
   * @param ms The millisecond timestamp of the event.
   * @param us The microsecond timestamp of the event.
   * @param device The input device that originated the event.
   */
  LTouchCancelEvent(UInt32 serial = LTime::nextSerial(),
                    UInt32 ms = LTime::ms(), UInt64 us = LTime::us(),
                    LInputDevice *device = nullptr) noexcept
      : LTouchEvent(LEvent::Subtype::Cancel, serial, ms, us, device) {}

 private:
  friend class LInputBackend;
  void notify();
};

#endif  // LTOUCHCANCELEVENT_H
