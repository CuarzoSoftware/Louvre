#ifndef LANIMATION_H
#define LANIMATION_H

#include <LCompositor.h>
#include <LObject.h>
#include <functional>
#include <LTime.h>

/**
 * @brief Time-based animations.
 *
 * An LAnimation can be used for creating graphical animations. It has a fixed duration in milliseconds, and is synchronized
 * with each output's refresh rate.\n
 * After started, the `onUpdate()` callback is triggered before each LOutput::paintGL() call, allowing you to
 * access the value() property, which is a 32-bit floating-point number linearly interpolated from 0.f to 1.f,
 * indicating the completion percentage of the animation.
 *
 * @note It is essential to manually invoke LOutput::repaint() on the outputs you are animating; otherwise, the `onUpdate()` callback may not be invoked.
 *
 * After the animation finishes, the `onFinish()` callback is triggered, and the value() property has a value of 1.f.\n
 */
class Louvre::LAnimation : public LObject
{
public:

    /**
     * Callback function type used to handle the `onUpdate()` and `onFinish()` events.
     */
    using Callback = std::function<void(LAnimation*)>;

    /**
     * @brief Creates a reusable animation.
     *
     * Creates an animation without starting it immediately.
     *
     * @param durationMs The duration of the animation in milliseconds.
     * @param onUpdate A callback function triggered each time the value() property changes. `nullptr` can be passed if not used.
     * @param onFinish A callback function triggered once the value() property reaches 1.f. `nullptr` can be passed if not used.
     */
    LAnimation(UInt32 durationMs = 0, const Callback &onUpdate = nullptr, const Callback &onFinish = nullptr);

    /**
     * @brief Destructor for the LAnimation class.
     *
     * Destroys an animation object. If the animation is currently running at the
     * time of destruction, the `onFinish()` callback is invoked immediately before
     * the object is deleted.
     */
    ~LAnimation();

    /// @cond OMIT
    LAnimation(const LAnimation&) = delete;
    LAnimation& operator= (const LAnimation&) = delete;
    /// @endcond

    /**
     * @brief Creates and launches a one-time animation with automatic cleanup.
     *
     * The oneShot() method creates and starts an animation immediately, and it is automatically destroyed once finished.
     *
     * @param durationMs The duration of the animation in milliseconds.
     * @param onUpdate A callback function triggered each time the value() property changes. `nullptr` can be passed if not used.
     * @param onFinish A callback function triggered once the value() property reaches 1.f. `nullptr` can be passed if not used.
     */
    inline static void oneShot(UInt32 durationMs, const Callback &onUpdate = nullptr, const Callback &onFinish = nullptr) noexcept
    {
        LAnimation *anim { new LAnimation(durationMs, onUpdate, onFinish) };
        anim->m_destroyOnFinish = true;
        anim->start();
    }

    /**
     * @brief Sets the `onUpdate()` callback handler function.
     *
     * This method allows you to set the callback function that will be called when an update event occurs.
     *
     * @param onUpdate A reference to the callback function. Pass `nullptr` to disable the callback.
     */
    inline void setOnUpdateCallback(const Callback &onUpdate) noexcept
    {
        if (m_running)
            return;

        m_onUpdate = onUpdate;
    }

    /**
     * @brief Sets the `onFinish()` callback handler function.
     *
     * This method allows you to set the callback function that will be called when the animaion finishes or stop() is called.
     *
     * @param onFinish A reference to the callback function. Pass `nullptr` to disable the callback.
     */
    inline void setOnFinishCallback(const Callback &onFinish) noexcept
    {
        if (m_running)
            return;

        m_onFinish = onFinish;
    }

    /**
     * @brief Sets the duration of the animation in milliseconds.
     *
     * Use this method to specify the duration of the animation in milliseconds.
     *
     * @note It is not permissible to invoke this method while the animation is in progress, and attempting to do so will yield no results.
     *
     * @param durationMs The duration of the animation in milliseconds.
     */
    inline void setDuration(UInt32 durationMs) noexcept
    {
        if (m_running)
            return;

        m_duration = durationMs;
    }

    /**
     * @brief Returns the duration of the animation in milliseconds.
     *
     * Use this method to retrieve the duration of the animation in milliseconds.
     *
     * @return The duration of the animation in milliseconds.
     */
    inline UInt32 duration() const noexcept
    {
        return m_duration;
    }

    /**
     * @brief Returns a number linearly interpolated from 0 to 1.
     *
     * This method returns a value indicating the percentage of completion of the animation. The value is linearly interpolated between 0 (start of the animation) and 1 (end of the animation).
     *
     * @return The interpolated completion value ranging from 0 to 1.
     */
    inline Float32 value() const noexcept
    {
        return m_value;
    }

    /**
     * @brief Starts the animation.
     */
    inline void start() noexcept
    {
        if (m_running)
            return;

        m_value = 0.f;
        m_beginTime = LTime::ms();
        m_running = true;
        compositor()->repaintAllOutputs();
    }

    /**
     * @brief Halts the animation before its duration is reached.
     *
     * The stop() method can be used to stop the animation before its duration is reached.
     * If called before the animation finishes, the `onFinish()` callback is triggered immediately, and the value() property is set to 1.
     */
    inline void stop()
    {
        if (!m_running)
            return;

        m_value = 1.f;
        m_running = false;

        if (m_onFinish)
            m_onFinish(this);

        if (m_destroyOnFinish)
            m_pendingDestroy = true;
    }

    /**
     * @brief Checks if the animation is currently running.
     *
     * @return `true` if running, `false` otherwise.
     */
    inline bool running() const noexcept
    {
        return m_running;
    }

private:
    friend class LCompositor;
    Callback m_onUpdate { nullptr };
    Float32 m_value { 0.f };
    UInt32 m_duration;
    UInt32 m_beginTime;
    bool m_running { false };
    bool m_processed { false };
    bool m_pendingDestroy { false };
    bool m_destroyOnFinish = false;
    Callback m_onFinish { nullptr };
};

#endif // LANIMATION_H
