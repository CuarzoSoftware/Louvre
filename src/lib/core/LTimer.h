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
    LTimer(const Callback &onTimeout = nullptr);

    /**
     * @brief Create a one-shot timer that is automatically started and destroyed on timeout or cancellation.
     * @param intervalMs The interval for the timer in milliseconds.
     * @param onTimeout The callback function to be executed when the timer expires.
     */
    static void oneShot(UInt32 intervalMs, const Callback &onTimeout);

    /// @cond OMIT
    LTimer(const LTimer&) = delete;
    LTimer& operator= (const LTimer&) = delete;
    /// @endcond

    /**
     * @brief Destroy the timer.
     *
     * Destroying a timer while it's running will not trigger its on timout callback.
     */
    void destroy();

    /**
     * @brief Set the callback function to be executed when the timer expires.
     *
     * @note The callback can't be changed while the timer is running.
     *
     * @param onTimeout The callback function to be executed when the timer expires.
     */
    void setCallback(const Callback &onTimeout);

    /**
     * @brief Retrieves the interval of the timer in milliseconds.
     * @return The interval of the timer.
     */
    UInt32 interval() const;

    /**
     * @brief Checks if the timer is currently running.
     * @return `true` if the timer is running, `false` otherwise.
     */
    bool running() const;

    /**
     * @brief Cancels the timer if it is currently running.
     *
     * If the timer is running and was created with oneShot() or started with destroyOnTimeout enabled,
     * the timer is destroyed and the onTimeout callback is not called. If the timer was started without
     * destroyOnTimeout, it can be safely started again and the onTimeout callback is also not called.
     */
    void cancel();

    /**
     * @brief Starts the timer with the specified interval and optional destroyOnTimeout flag.
     *
     * If the timer was created with destroyOnTimeout disabled, it is safe to call start() again within
     * the onTimeout callback.
     *
     * @note Calling start() on an already started timer is a no-op. If no callback function is set before calling
     *       this method the timer doesn't start, this method returns `false` and the timer is not destroyed if destroyOnTimeout
     *       was enabled.
     *
     * @param intervalMs The interval for the timer in milliseconds.
     * @param destroyOnTimeout If  `true `, the timer will be destroyed after the timeout callback.
     *
     * @return `true` if the timer successfully started, `false` otherwise.
     */
    bool start(UInt32 intervalMs, bool destroyOnTimeout = false);

LPRIVATE_IMP_UNIQUE(LTimer)
    ~LTimer();
};

#endif // LTIMER_H
