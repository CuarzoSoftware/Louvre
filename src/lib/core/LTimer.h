#ifndef LTIMER_H
#define LTIMER_H

#include <LObject.h>
#include <functional>

/**
 * @brief Delayed callbacks
 *
 * This class provides the capability to create and manage timers, allowing you to schedule time intervals
 * in milliseconds and execute a specified callback function when the timer expires. It supports both
 * re-usable timers and one-shot timers that can be automatically destroyed after timeout or cancellation.
 */
class Louvre::LTimer : public LObject
{
public:
    /**
     * @brief Type definition for the timer's timeout callback function.
     * @param timer Pointer to the LTimer instance that triggered the callback.
     */
    using Callback = std::function<void(LTimer*)>;

    /**
     * @brief Construct a re-usable LTimer instance with the provided timeout callback.
     * @param onTimeout The callback function to be executed when the timer expires.
     */
    LTimer(const Callback &onTimeout = nullptr) noexcept;

    /**
     * @brief Destructor for the LTimer class.
     *
     * If the timer is destroyed while it is running, the associated callback function is not invoked.
     */
    ~LTimer();

    /**
     * @brief Create a one-shot timer that is automatically started and destroyed on timeout or cancellation.
     * @param intervalMs The interval for the timer in milliseconds.
     * @param onTimeout The callback function to be executed when the timer expires.
     *
     * @return `true` on successfully, `false` otherwise.
     */
    static bool oneShot(UInt32 intervalMs, const Callback &onTimeout) noexcept;

    /// @cond OMIT
    LTimer(const LTimer&) = delete;
    LTimer& operator= (const LTimer&) = delete;
    /// @endcond

    /**
     * @brief Set the callback function to be executed when the timer expires.
     *
     * @param onTimeout The callback function to be executed when the timer expires.
     */
    inline void setCallback(const Callback &onTimeout) noexcept
    {
        m_onTimeoutCallback = onTimeout;
    }

    /**
     * @brief Retrieves the interval of the timer in milliseconds.
     * @return The interval of the timer.
     */
    inline UInt32 interval() const noexcept
    {
        return m_interval;
    }

    /**
     * @brief Checks if the timer is currently running.
     * @return `true` if the timer is running, `false` otherwise.
     */
    inline bool running() const noexcept
    {
        return m_running;
    }

    /**
     * @brief Cancels the timer without invoking the callback.
     *
     * If the timer was created with oneShot() it is destroyed and the onTimeout callback is not called.
     */
    void cancel() noexcept;

    /**
     * @brief Stops the timer invoking the callback immediatly.
     *
     * If the timer was created with oneShot() it is destroyed after the callback is triggered.
     */
    void stop() noexcept;

    /**
     * @brief Starts the timer with the specified interval.
     *
     * @note Calling start() on an already started timer will restart the timer without calling the callback.
     *       If this method is called within the timeout callback the timer is restarted even if it was created
     *       using oneShot().
     *
     * @param intervalMs The interval for the timer in milliseconds.
     *
     * @return `true` if the timer successfully started, `false` otherwise.
     */
    bool start(UInt32 intervalMs) noexcept;

private:
    Callback m_onTimeoutCallback;
    wl_event_source *m_waylandEventSource { nullptr };
    UInt32 m_interval { 0 };
    bool m_running { false };
    bool m_destroyOnTimeout { false };
    static Int32 waylandTimeoutCallback(void *data) noexcept;
};

#endif // LTIMER_H
