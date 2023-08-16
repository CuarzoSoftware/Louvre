#ifndef LTIMER_H
#define LTIMER_H

#include <LObject.h>
#include <functional>

using namespace Louvre;

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
     * @brief Constructs a re-usable LTimer instance with the provided timeout callback.
     * @param onTimeout The callback function to be executed when the timer expires.
     */
    LTimer(const Callback &onTimeout);

    /**
     * @brief Creates a one-shot timer that is automatically started and destroyed on timeout or cancellation.
     * @param intervalMs The interval for the timer in milliseconds.
     * @param onTimeout The callback function to be executed when the timer expires.
     */
    static void oneShot(UInt32 intervalMs, const Callback &onTimeout);

    LTimer(const LTimer&) = delete;
    LTimer& operator= (const LTimer&) = delete;

    /**
     * @brief Destructor for the LTimer class.
     */
    ~LTimer();

    /**
     * @brief Sets the callback function to be executed when the timer expires.
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
     * destroyOnTimeout, it can be safely started again.
     */
    void cancel();

    /**
     * @brief Starts the timer with the specified interval and optional destroyOnTimeout flag.
     *
     * If the timer was created with destroyOnTimeout disabled, it is safe to call start() again within
     * the onTimeout callback.
     *
     * @note Calling start() on a already started timer is a no-op.
     *
     * @param intervalMs The interval for the timer in milliseconds.
     * @param destroyOnTimeout If true, the timer will be destroyed after the timeout callback.
     */
    void start(UInt32 intervalMs, bool destroyOnTimeout = false);

LPRIVATE_IMP(LTimer)
};

#endif // LTIMER_H
