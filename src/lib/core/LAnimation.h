#ifndef LANIMATION_H
#define LANIMATION_H

#include <LCompositor.h>
#include <LObject.h>
#include <functional>

/*!
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
 *
 * To create an LAnimation, you can use the oneShot() or create() methods.\n
 *
 * The oneShot() method creates and starts an animation immediately, and it is automatically destroyed once finished.
 *
 * #### Example using oneShot()
 * \code{.cpp}
 *  LAnimation::oneShot(5000,
 *
 *   // On Update
 *   [yourObjectToAnimate](LAnimation *anim)
 *   {
 *       yourObjectToAnimate->setPos(anim->value() * x);
 *   },
 *
 *   // On Finish
 *   [yourObjectToAnimate](LAnimation *anim)
 *   {
 *       yourObjectToAnimate->hide();
 *   });
 * \endcode
 *
 * On the other hand, the create() method generates an LAnimation that can be reused multiple times.\n
 * You need to call the start() method to initiate it. By passing `true` to start(), the animation is automatically
 * destroyed upon completion, whereas passing `false` keeps the animation intact for potential reuse.
 *
 * #### Example using create()
 * \code{.cpp}
 *  LAnimation *animation = LAnimation::create(5000,
 *
 *   // On Update
 *   [yourObjectToAnimate](LAnimation *anim)
 *   {
 *       yourObjectToAnimate->setPos(anim->value() * x);
 *   },
 *
 *   // On Finish
 *   [yourObjectToAnimate](LAnimation *anim)
 *   {
 *       yourObjectToAnimate->hide();
 *   });
 *
 * // Starts the animation without destroying it on finish
 * animation->start(false);
 * \endcode
 *
 * ### Common Mistakes
 * @warning Always ensure that the objects you animate remain valid during the animation to avoid potential segmentation faults. A recommended practice is to use the create() method and call stop() if an animated object is destroyed before it finishes.
 */
class Louvre::LAnimation : public LObject
{
public:

    /*!
     * Callback function type used in "onUpdate" and "onFinish".
     */
    using Callback = std::function<void(LAnimation*)>;

    /// @cond OMIT
    LAnimation(const LAnimation&) = delete;
    LAnimation& operator= (const LAnimation&) = delete;
    /// @endcond

    /*!
     * @brief Creates and launches a one-time animation with automatic cleanup.
     *
     * The oneShot() method creates and starts an animation immediately, and it is automatically destroyed once finished.
     *
     * @param durationMs The duration of the animation in milliseconds.
     * @param onUpdate A callback function triggered each time the value() property changes. `nullptr` can be passed if not used.
     * @param onFinish A callback function triggered once the value() property reaches 1.f. `nullptr` can be passed if not used.
     */
    static void oneShot(UInt32 durationMs, const Callback &onUpdate = nullptr, const Callback &onFinish = nullptr);

    /*!
     * @brief Creates a reusable animation.
     *
     * The create() method creates an animation without starting it immediately.
     *
     * @param durationMs The duration of the animation in milliseconds.
     * @param onUpdate A callback function triggered each time the value() property changes. `nullptr` can be passed if not used.
     * @param onFinish A callback function triggered once the value() property reaches 1.f. `nullptr` can be passed if not used.
     */
    static LAnimation *create(UInt32 durationMs, const Callback &onUpdate = nullptr, const Callback &onFinish = nullptr);

    /*!
     * @brief Sets the `onUpdate()` callback handler function.
     *
     * This function allows you to set the callback function that will be called when an update event occurs.
     *
     * @param onUpdate A reference to the callback function. Pass `nullptr` to disable the callback.
     */
    void setOnUpdateCallback(const Callback &onUpdate);

    /*!
     * @brief Sets the `onFinish()` callback handler function.
     *
     * This function allows you to set the callback function that will be called when the animaion finishes or stop() is called.
     *
     * @param onFinish A reference to the callback function. Pass `nullptr` to disable the callback.
     */
    void setOnFinishCallback(const Callback &onFinish);

    /*!
     * @brief Sets the duration of the animation in milliseconds.
     *
     * Use this method to specify the duration of the animation in milliseconds. It's important to note that this method
     * should not be called while the animation is running.
     * If called while the animation is running, it will have no effect (no-op).
     *
     * @param durationMs The duration of the animation in milliseconds.
     */
    void setDuration(UInt32 durationMs);

    /*!
     * @brief Returns the duration of the animation in milliseconds.
     *
     * Use this function to retrieve the duration of the animation in milliseconds.
     *
     * @return The duration of the animation in milliseconds.
     */
    UInt32 duration() const;

    /*!
     * @brief Returns a number linearly interpolated from 0 to 1.
     *
     * This function returns a value indicating the percentage of completion of the animation. The value is linearly interpolated between 0 (start of the animation) and 1 (end of the animation).
     *
     * @return The interpolated completion value ranging from 0 to 1.
     */
    Float32 value() const;

    /*!
     * @brief Starts the animation.
     *
     * Use this function to initiate the animation. You can also specify whether to destroy the animation object after the `onFinish` callback is called by providing a boolean parameter.
     *
     * @param destroyOnFinish If set to true, the animation is destroyed after `onFinish()` is called. Default is true.
     */
    void start(bool destroyOnFinish = true);

    /*!
     * @brief Halts the animation before its duration is reached.
     *
     * The stop() method can be used to stop the animation before its duration is reached.
     * If called before the animation finishes, the `onFinish()` callback is triggered immediately, and the value() property is set to 1.
     */
    void stop();

    /*!
     * @brief Destroys the animation without invoking `onFinish()`.
     */
    void destroy();

LPRIVATE_IMP(LAnimation)
    /// @cond OMIT
    friend class Louvre::LCompositor::LCompositorPrivate;
    LAnimation();
    ~LAnimation();
    /// @endcond
};

#endif // LANIMATION_H
