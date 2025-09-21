#ifndef LIDLELISTENER_H
#define LIDLELISTENER_H

#include <LObject.h>

/**
 * @brief Idle state listener
 *
 * @anchor lidlelistener_detailed
 *
 * Clients using the [Idle
 * Notify](https://wayland.app/protocols/ext-idle-notify-v1) protocol create an
 * instance of this class to be notified when the user has been idle for a set
 * amount of time. For example,
 * [swayidle](https://man.archlinux.org/man/swayidle) uses this information to
 * trigger screen lockers, screen savers, etc.
 *
 * @note All idle listeners can be accessed from LSeat::idleListeners().
 *
 * Each listener has its own timer with a fixed timeout() in milliseconds,
 * defined by the client(). The timer should be reset with resetTimer() whenever
 * a user event occurs (see LSeat::onEvent()).
 *
 * If there is no user activity and the timeout() is reached,
 * LSeat::onIdleListenerTimeout() is triggered.
 *
 * - If resetTimer() is not called within that event, the client will be
 * notified that the user is idle until resetTimer() is called again.
 * - If a client requests to inhibit the idle state (see section below),
 * resetTimer() should always be called.
 *
 * @note Resetting all timers each time an event occurs isn't very CPU-friendly.
 * Consider using LSeat::setIsUserIdleHint() instead like the default
 * LSeat::onEvent() implementation does.
 *
 * @section idle_inhibitors Idle Inhibitors
 *
 * Clients using the [Idle
 * Inhibit](https://wayland.app/protocols/idle-inhibit-unstable-v1) protocol can
 * request to prevent the idle state if one of their surfaces is mapped and
 * visible.
 *
 * These surfaces can be accessed from LSeat::idleInhibitorSurfaces().
 *
 * The auxiliary virtual method LSeat::isIdleStateInhibited() is used by the
 * default implementation of LSeat::onIdleListenerTimeout() to check if the
 * timer should be reset.
 */
class Louvre::LIdleListener final : public LObject {
 public:
  /**
   * @brief Resets the internal timer.
   *
   * This method should be called each time an event indicating user activity
   * occurs (see LSeat::onEvent()). When the timeout() is reached,
   * LSeat::onIdleListenerTimeout() is triggered.
   */
  void resetTimer() const noexcept;

  /**
   * @brief The timeout in milliseconds.
   */
  UInt32 timeout() const noexcept;

  /**
   * @brief Client owner of the listener.
   */
  LClient *client() const noexcept;

  /**
   * @brief Associated Wayland resource.
   */
  const Protocols::IdleNotify::RIdleNotification &resource() const noexcept {
    return m_resource;
  }

 private:
  friend class Protocols::IdleNotify::RIdleNotification;
  LIdleListener(Protocols::IdleNotify::RIdleNotification &resource) noexcept;
  ~LIdleListener() noexcept;
  Protocols::IdleNotify::RIdleNotification &m_resource;
};

#endif  // LIDLELISTENER_H
