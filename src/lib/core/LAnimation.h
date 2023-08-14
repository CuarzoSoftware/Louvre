#ifndef LANIMATION_H
#define LANIMATION_H

#include <LObject.h>
#include <functional>

/*!
 * @brief Time-based animations.
 *
 * An LAnimation can be used to animate object positions, colors, opacity, etc. It has a fixed duration in milliseconds.
 * Once started, an onUpdate callback is triggered,
 * allowing you to access the value() property, which is a 32-bit floating-point number linearly interpolated from 0.f to 1.f.\n
 * After the animation finishes, the onFinish callback is triggered, and the value() property is set to 1.f.\n
 * To create an LAnimation, you can use the oneShot() or create() methods.\n
 * The oneShot() method creates and starts an animation immediately, and it is automatically destroyed once finished.\n
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
 * On the other hand, the create() method generates an LAnimation that can be reused multiple times.
 * You need to call the start() method to initiate it. By passing 'true' to start(), the animation is automatically
 * destroyed upon completion, whereas passing 'false' keeps the animation intact for potential reuse.
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
 * #### Common mistakes
 * @warning Always ensure that the objects you animate remain valid during the animation to avoid potential segmentation faults. A recommended practice is to use the create() method and call stop() if the object is destroyed before the animation finishes.
 */
class Louvre::LAnimation : public LObject
{
public:

    /*!
     * Callback function type used in "onUpdate" and "onFinish".
     */
    using Callback = std::function<void(LAnimation*)>;

    LAnimation(const LAnimation&) = delete;
    LAnimation& operator= (const LAnimation&) = delete;

    /*!
     * Destructor of the LAnimation class.
     */
    ~LAnimation();

    /*!
     * The oneShot() method creates and starts an animation immediately, and it is automatically destroyed once finished.
     *
     * @param durationMs The duration of the animation in milliseconds.
     * @param onUpdate A callback function triggered each time the value() property changes. nullptr can be passed if not used.
     * @param onFinish A callback function triggered once the value() property reaches 1.f. nullptr can be passed if not used.
     */
    static void oneShot(UInt32 durationMs, const Callback &onUpdate = nullptr, const Callback &onFinish = nullptr);

    /*!
     * The create() method creates an animation without starting it immediately.
     *
     * @param durationMs The duration of the animation in milliseconds.
     * @param onUpdate A callback function triggered each time the value() property changes. nullptr can be passed if not used.
     * @param onFinish A callback function triggered once the value() property reaches 1.f. nullptr can be passed if not used.
     */
    static LAnimation *create(UInt32 durationMs, const Callback &onUpdate = nullptr, const Callback &onFinish = nullptr);

    /*!
     * Sets the onUpdate callback handler function. nullptr can be passed to disable it.
     */
    void setOnUpdateCallback(const Callback &onUpdate);

    /*!
     * Sets the onFinish callback handler function. nullptr can be passed to disable it.
     */
    void setOnFinishCallback(const Callback &onFinish);

    /*!
     * Sets the duration of the animation in milliseconds.
     * This method should not be called while the animation is running.
     * If called while the animation is running, it will have no effect (no-op).
     */
    void setDuration(UInt32 durationMs);

    /*!
     * Returns the duration of the animation in milliseconds.
     */
    UInt32 duration() const;

    /*!
     * Returns a number linearly interpolated from 0 to 1, indicating the percentage of completion of the animation.
     */
    Float32 value() const;

    /*!
     * Starts the animation.
     *
     * @param destroyOnFinish If set to true, the animation is destroyed after onFinish() is called.
     */
    void start(bool destroyOnFinish = true);

    /*!
     * The stop method can be used to halt the animation before its duration is reached.\n
     * If called before the animation finishes, the onFinish callback is triggered immediately, and the value() property is set to 1.
     */
    void stop();

LPRIVATE_IMP(LAnimation)
    LAnimation();
};

#endif // LANIMATION_H
