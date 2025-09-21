#ifndef LKEYBOARDENTEREVENT_H
#define LKEYBOARDENTEREVENT_H

#include <LKeyboardEvent.h>
#include <LTime.h>

/**
 * @brief Event generated when a surface or view gains keyboard focus.
 */
class Louvre::LKeyboardEnterEvent final : public LKeyboardEvent {
 public:
  /**
   * @brief Constructor for LKeyboardEnterEvent.
   *
   * @param serial The serial number of the event.
   * @param ms The millisecond timestamp of the event.
   * @param us The microsecond timestamp of the event.
   * @param device The input device that originated the event.
   */
  LKeyboardEnterEvent(UInt32 serial = LTime::nextSerial(),
                      UInt32 ms = LTime::ms(), UInt64 us = LTime::us(),
                      LInputDevice *device = nullptr) noexcept
      : LKeyboardEvent(LEvent::Subtype::Enter, serial, ms, us, device) {}
};

#endif  // LKEYBOARDENTEREVENT_H
