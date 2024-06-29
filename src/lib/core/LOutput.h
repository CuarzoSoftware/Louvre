#ifndef LOUTPUT_H
#define LOUTPUT_H

#include <LFactoryObject.h>
#include <LSize.h>
#include <LRect.h>
#include <LRegion.h>
#include <LFramebuffer.h>
#include <LContentType.h>

#include <thread>
#include <list>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <sys/eventfd.h>
#include <poll.h>

/**
 * @brief A display rendering interface.
 *
 * @anchor loutput_detailed
 *
 * The LOutput class is responsible for rendering content to a display. It is typically associated with a physical screen, but could also
 * represent a toplevel window within Wayland desktop, depending on the selected graphic backend.
 *
 * <center><IMG height="250px" SRC="https://lh3.googleusercontent.com/4lV1LTHBmO-eFywBrL4UhYIRcQbV5bjGB_17FdWFCzjGvnklxwBnXz5hQKOrkRCOegsn6PjnYZNCWk1SjFjwh9t8olEzr3Uwzd3saEt8EKRbbqX0n1f5R7q6r6V9u1t0PUk7BB0teA"></center>
 *
 * @section Lifetime
 *
 * During compositor initialization, the graphic backend creates an LOutput for each available display through LCompositor::createObjectRequest().\n
 * These can be accessed via the LSeat::outputs() vector.
 *
 * After the compositor has been initialized, the number of available outputs can change. These changes are notified through the LSeat::outputPlugged() and LSeat::outputUnplugged() events.
 *
 * @section Initialization
 *
 * By default, all outputs are uninitialized. To initialize an output, use LCompositor::addOutput(), as done in the default implementation of
 * LCompositor::initialized() and LSeat::outputPlugged().
 *
 * When an output is successfully initialized, it is added to LCompositor::outputs() and a new output-specific rendering thread and shared OpenGL context
 * are created, triggering the initializeGL() event.
 *
 * @section Uninitialization
 *
 * To uninitialize an output, use LCompositor::removeOutput(). This will invoke the uninitializeGL() event, destroy the rendering thread and OpenGL context
 * and remove the output from LCompositor::outputs().\n
 *
 * @note Outputs are also automatically uninitialized following an LSeat::outputUnplugged() event.
 *
 * @section Rendering
 *
 * The rendering thread loop remains blocked until repaint() is called, which unlocks the thread and triggers the paintGL() event.
 *
 * All painting operations must occur exclusively within a paintGL() event, as rendering elsewhere will not be visible on the screen.
 *
 * Calling repaint() multiple times will only unlock the thread once. After or during a paintGL() event, repaint() must be called again to unlock the thread once more.
 *
 * @section Painting
 *
 * Each output has its own OpenGL context and its own instance of LPainter, accessible via painter().
 *
 * You can use LPainter methods to render colored rectangles or textures, use OpenGL directly with your own shaders/programs, or rely on the LScene class.\n
 * LScene can render views optimally by considering surface damage, opaque regions, and other factors to significantly improve performance.
 *
 * @section Modes
 *
 * Each LOutput has at least one mode. An LOutputMode contains information about the resolution and refresh rate at which the output can operate.
 *
 * You can access the modes of an output using modes() and set the desired one with setMode().
 *
 * By default, outputs use the preferredMode(), which typically has the highest refresh rate and resolution.
 *
 * If you change an output's mode or scale while it is initialized, the resizeGL() event is triggered.
 *
 * @section Arrangement
 *
 * Outputs, like surfaces, can be positioned within the compositor-global coordinate space similarly to how a system settings panel operates.
 *
 * You can adjust the position of an output using setPos(). This will later trigger the moveGL() event if the position changes.
 *
 * <center><IMG height="350px" SRC="https://lh3.googleusercontent.com/VOWUX4iiqYMF_bIrBP3xMyaiydv_e_ZKznCIJlRLaEA0CtBLMuU4h41R3D4Xm-7krk8jFGZrQGb_SS7hlIFUY9E5dVbQqs0Q3NIBXvRFrGs_cukqOmbCv1ExN9fG3BDdj4Yz45xIkQ=w2400"></center>
 *
 * @note To enable LCursor to transition across different LOutputs, ensure that the outputs are closely arranged side by side.
 *
 * @section Scaling
 *
 * Many screens nowadays are HiDPI, so it is commonly required to apply a scaling factor to prevent content from appearing tiny on the screen.\n
 * By default, all outputs have a scaling factor of 1, meaning no scaling is applied. To assign the scale factor use setScale(),
 * which modifies the size returned by size() (same as `rect().size()`).
 *
 * In Louvre, you typically need to deal with two type of units: ***buffer units*** and ***surface units***.
 *
 * In buffer units, the scale factor is not taken into account, and the dimensions always have maximum granularity.\n
 * For example, if a screen has a resolution of 2000x1000px, its size in buffer units would be the same: 2000x1000px, which is returned by sizeB().\n
 * However, the compositor-global coordinate space uses surface units, which is equal to the buffer size divided by the applied scale factor.\n
 * Therefore, if a scale factor of 2 applied to the screen, its size in surface units would be 1000x500, which is returned by size() and `rect().size()`.
 *
 * @note Use surface units (pos() and size() or rect()) when arranging outputs.
 *
 * Let's look at an example to make these concepts clearer:
 *
 * Let's assume you have two displays, one with a resolution of 1000x500px and another with 2000x1000px (double the resolution), but both with a physical size of 22''.
 * This means that the space occupied by 1px on the blue screen accommodates 4px on the pink screen.
 *
 * If you were to see them in real life, side by side they would look like this:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV87QBEaAMVfuF92jfv0HRQrEgc0PMKgx9N29Wb6dDbF9cHLpCr7qSDUowUFnBXFHJxg4F9c7v7EcxxTSnpSzkEqLvCB7CxlnUYJmG1JsNspSHRq3zZE=w2400"></center>
 *
 * If you assign the same scaling factor of 1 to both screens, their sizes in surface units would be the same as in buffer units.\n
 * Therefore, from the compositor-global coordinate space point of view they would be arranged like this:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV86CZvhAIc-yONr5BQe6ABGxfcuSVajo1jobAWL5C6tMdJO8BXjsbMbZk7r7rEZ9-ZsYmr3LsIdBSKkb3npEYoCIG1OD0iZKUoCDFVoqMG4avt06Vhs=w2400"></center>
 *
 * And if you were to see them in real life, they would look like the following. In the pink screen, everything would appear tiny, half the size of the blue screen.
 * And as you can see, if you were to drag an application window from one screen to another, it would look somewhat odd.
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV84F3o3dewpcSRttqMHPjKO2YYUqSJ299tj760KcT6KRd0s7uf_bXtKQfwy5CFeXqPoxynuu2UmtkEOodg1l7DjZHoXohjXdpGIth9S50mKGdsHqfPw=w2400"></center>
 *
 * Now, let's imagine that you assign a scaling factor of 2 to the pink screen. In this case, the compositor-global coordiante space would look as follows:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV86j52vqCYAxcm94frzQHzeI47idYlb-tggnlbwcVsZZwXIkr9M4tyhKPMDLfmtjOYqHWS9jnCiXojuusl8EKyv3OIn8KQX6biHr0hQeLxH7m04VaTc=w2400"></center>
 *
 * And therefore, if you were to see them in real life now, the UI elements dimensions would appear consistently.
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV87mN6ufD6PY9Rt8fjl3B84ii627PdOYwJx9vmeA72EeT_cpr8Q01IdoiBknoPI6fonVJWRCW34VZNKVKCsNIERY70Gvqkl7eH0R2vr3gBrEtuMNps0=w2400"></center>
 *
 * Therefore, in summary, the size of a screen in surface units is determined by dividing its buffer size by the applied scale factor.
 *
 * ## Fractional Scaling
 *
 * Louvre also supports fractional scaling, albeit with some differences compared to integer scaling. For instance, if you assign a scale factor of 1.5 using setScale()
 * to the pink screen, the resulting applied scale (obtained with scale()) will be ceil(1.5) = 2. However, the buffer dimensions of the screen will simulate
 * being 2/1.5 times larger than its current mode (rounded), resulting in 2668 x 1334 for this case. Consequently, its size in surface coordinates would be
 * 2668 x 1334 divided by 2.
 * As a result, the compositor-global coordinate space would appear as follows:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV86R-d2uzDybJ-FLWL5PzbShg33f2_OJX9mQME8DkVI5y6O3eDT18UBRgwY8UScaApeoqU4ePq3BwbDNmW5Z-eW0dukpvdQfmarUbOVEbN_GmYU3MqE=w2400"></center>
 *
 * This creates the illusion of rendering on a larger screen, the result of which is actually scaled to the real size, allowing for the desired scaling effect.\n
 * Rendering using fractional scaling, however, can introduce undesired visual effects like aliasing, especially noticeable when moving elements with textures containing fine details.
 * For this reason, Louvre offers the option to render using oversampling, where all the screen content is rendered into a larger buffer, and then that rendered buffer is scaled down to the actual screen framebuffer.
 * This method almost completely eliminates aliasing but has the disadvantage of consuming more computational power, potentially decreasing performance.
 * Without oversampling the content is directly rendered on the screen, making it efficient but retaining aliasing artifacts.\n
 * Louvre allows you to toggle oversampling on and off instantly at any time using enableFractionalOversampling().
 * For example, you could enable it when displaying a desktop with floating windows and disable it when displaying a fullscreen window.
 *
 * @note Oversampling is not required and is always disabled when using non-fractional scales. Therefore, as a recommendation, if your monitor supports multiple modes() with various resolutions,
 *       it is preferable to select one of those modes instead of using fractional scaling.
 *
 * Clients supporting the [fractional scaling protocol](https://wayland.app/protocols/fractional-scale-v1) are instructed to scale their buffers directly to the fractional scale.\n
 * On the other hand, clients lacking support for the protocol are advised to use ceil(fractional scale), ensuring a consistently high-detail appearance.
 *
 * @section Transforms
 *
 * Louvre supports applying transforms to outputs with setTransform().
 *
 * Let's imagine that you physically rotate the pink monitor 90° clockwise while maintaining the same normal transform on both.
 * What you would see is the following:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV85bV4h-OcbW1DozgUCQp4PbMoWOd2E2MNRQveEhw5r2p_mjnVRLbziTJ-egMu9D0eSIbMyLNmXECbupfa1gmQZMAidIS1pT5ahbeoL6hcsa6O3QlYc=w2400"></center>
 *
 * Given that you rotated the screen 90° clockwise, it is necessary to apply a transform that rotates the screen 90° counter-clockwise (@ref LTransform::Rotated90).
 * If you apply this to the pink screen, then you would see the following:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV84Tm3VGwuNHqbAumAVrzZVgBtKSOo24y8PDk5Y47hIlLmO7uSzIMLeE0f9y-5DOQh1nOI9-qz48c_fnlgkM3CILC-GFB2qttGaE671Xke_jcF6DlVY=w2400"></center>
 *
 * Note that when applying a transformation containing a 90° or 270° rotation to an output, the components of sizeB(), size() and `rect().size()` are swapped (their width becomes the
 * height, and the height becomes the width).
 * The compositor-global coordiante space is structured in such a way that you can continue rendering in the same manner
 * as if the screens were in their normal transform state. Therefore, there's no need to worry about rotating or flipping the elements you draw unless you decide not to
 * use LPainter or LScene for rendering.
 *
 * @section VSync
 *
 * All outputs have VSync enabled by default. Disabling VSync may not always be supported, refer to hasVSyncControlSupport().
 *
 * VSync can be toggled using enableVSync(). When VSync is disabled, Louvre limits the frame rate to double the refresh rate of
 * the currentMode() by default. This setting can be changed with setRefreshRateLimit().
 *
 * Clients using the Tearing Protocol can indicate their preference for each individual surface.\n
 * See LSurface::preferVSync() and LSurface::preferVSyncChanged() for more details.
 */
class Louvre::LOutput : public LFactoryObject
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LOutput;

    /**
     * @brief Output state.
     *
     * @see state()
     */
    enum State
    {
        PendingInitialize,   ///< Output is pending initialization.
        PendingUninitialize, ///< Output is pending uninitialization.
        Initialized,         ///< Output is initialized and active.
        Uninitialized,       ///< Output is uninitialized.
        ChangingMode,        ///< Output is in the process of changing display mode.
        Suspended            ///< Output is suspended.
    };

    /**
     * @brief Subpixel geometry.
     *
     * This enumeration provides information about how the physical pixels on an output are laid out.
     *
     * @see subPixel()
     */
    enum SubPixel
    {
        Unknown         = 0, ///< Unknown geometry.
        None            = 1, ///< No specific geometry.
        HorizontalRGB   = 2, ///< Horizontal RGB layout.
        HorizontalBGR   = 3, ///< Horizontal BGR layout.
        VerticalRGB     = 4, ///< Vertical RGB layout.
        VerticalBGR     = 5  ///< Vertical BGR layout.
    };

    /**
     * @brief Constructor of the LOutput class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LOutput(const void *params) noexcept;

    /**
     * @brief Destructor of the LOutput class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LOutput();

    LCLASS_NO_COPY(LOutput)

    /**
     * @brief Gets the current state of the LOutput.
     *
     * This method returns the current state of the output.
     */
    State state() const noexcept;

    /**
     * @brief Gets access to the associated LPainter.
     *
     * This method provides access to the LPainter associated with this output.
     */
    LPainter *painter() const noexcept;

    /**
     * @brief Session Lock Surface
     *
     * When a client requests to lock the user session (see LSessionLockManager), it creates an LSessionLockRole surface for each initialized output whith
     * a size equals to the output size().
     *
     * If the session is locked and there is no LSessionLockRole assigned to this output, the compositor should still avoid rendering any user-related content.
     *
     * @return An instance of the LSessionLockRole associated with this output if the session is locked, or `nullptr` if the session is not locked or the locking client didn't create one.
     */
    LSessionLockRole *sessionLockRole() const noexcept;

    /**
     * @brief Retrieves all exclusive zones assigned to this output.
     *
     * The order of the list determines how zones anchored to the same edge are stacked.
     * Zones listed first are positioned closer to the output's edge. Additionally,
     * the order affects their predominance: zones listed later adjust their space to
     * avoid occluding those listed earlier.
     *
     * @return A list of pointers to LExclusiveZone objects.
     * @see LExclusiveZone::setOutput()
     */
    const std::list<LExclusiveZone*> exclusiveZones() const noexcept;

    /**
     * @brief Retrieves the rect within the output that is not occupied by exclusive zones.
     *
     * The available geometry defines the space within an output where, for example,
     * LToplevelRole surfaces should be constrained to prevent occluding UI elements like a panel.
     *
     * The rect is in output-local surface coordinates.
     *
     * @see exclusiveZones()
     *
     * @return A rect representing the available geometry.
     */
    const LRect &availableGeometry() const noexcept;

    /**
     * @brief Gets the sum of the space occupied by exclusive zones for each edge.
     *
     * This function provides margins that represent the total area taken up by the exclusive zones
     * along each edge of the output.
     *
     * @see exclusiveZones()
     * @see availableGeometry()
     *
     * @return LMargins object representing the exclusive edges.
     */
    const LMargins &exclusiveEdges() const noexcept;

    /**
     * @brief Returns a pointer to the associated framebuffer.
     *
     * @see LPainter::bindFramebuffer()
     *
     * @return A pointer to the LFramebuffer instance associated with the output.
     */
    LFramebuffer *framebuffer() const noexcept;

    /**
     * @brief Gets the framebuffer transformation.
     *
     * This method returns the current framebuffer transformation applied with setTransform().
     *
     * The default value is @ref LTransform::Normal.
     *
     * @return The framebuffer transformation.
     */
    LTransform transform() const noexcept;

    /**
     * @brief Sets the framebuffer transform.
     *
     * This method sets the output transform, allowing you to adjust the orientation/flipping of the output.\n
     * If the specified transform includes a 90 or 270-degree rotation, the width and height of the output are swapped accordingly.
     *
     * @param transform The framebuffer transformation to apply.
     */
    void setTransform(LTransform transform) noexcept;

    /**
     * @brief Returns the index of the current buffer.
     *
     * Compositors commonly employ double or triple buffering. This involves rendering to one buffer while displaying another, prventing visual artifacts like tearing.
     *
     * @image html https://lh3.googleusercontent.com/2ousoWwxnVGvFX5bT6ual2G8UUbhUOJ21mK1UQmthPNM-7XfracRlL5GCYBQTzt4Os28eKO_FzC6BS-rasiNngvTMI9lEdET0ItKrI2wK_9IwSDaF-hNGkTMI6gVlL0m4ENDJYbckw
     *
     * @return The current buffer index. Alternates between [0], [0, 1] or [0, 1, 2] depending on the graphic backend configuration.
     */
    Int32 currentBuffer() const noexcept;

    /**
     * @brief Returns the count of available buffers.
     *
     * This method returns the number of buffers used by the output. It can be 1, 2 or 3 depending on the graphic backend configuration.
     */
    UInt32 buffersCount() const noexcept;

    /**
     * @brief Access the texture of a specific buffer.
     *
     * This method allows access to the texture associated with a particular buffer index.
     *
     * @param bufferIndex The index of the buffer for which the texture is to be accessed.
     * @return A pointer to the texture associated with the specified buffer index, or `nullptr` if texture access is not supported.
     *
     * @warning Some hardware/backends may not support accessing outputs textures, so you should always check if `nullptr` is returned.
     */
    LTexture *bufferTexture(UInt32 bufferIndex) noexcept;

    /**
     * @brief Checks if the output supports buffer damage tracking.
     *
     * Some graphic backends/hardware can benefit from knowing which regions of the framebuffer have changed within a paintGL() event.
     *
     * This method indicates whether buffer damage support is available.
     *
     * @see setBufferDamage() and bufferDamage()
     *
     * @return `true` if the graphical backend supports buffer damage tracking, `false` otherwise.
     */
    bool hasBufferDamageSupport() const noexcept;

    /**
     * @brief Specifies the damage generated during the last painGL() event.
     *
     * The bufferDamage() is automatically reset just before the subsequent paintGL() event.
     *
     * @note Although calling this method is not mandatory, it can significantly enhance performance, especially on certain graphic backends/hardware
     *       or when the output scale is fractional and oversampling is enabled. If never invoked, the entire output is considered damaged.
     *
     * When using LScene for rendering, damage calculation is handled automatically with this method being called inside LScene::handlePaintGL().
     *
     * @param damage The damaged region of the output in compositor-global coordinates.
     *               Providing an empty region indicates no damage, while passing `nullptr` implies the entire output is damaged.
     */
    void setBufferDamage(const LRegion *damage) noexcept;

    /**
     * @brief Retrieves the damage region set by setBufferDamage().
     *
     * Before each paintGL() event, the compositor automatically clears the damage region and marks the entire output
     * as damaged by adding rect().
     *
     * @return The damage region in compositor-global coordiantes.
     */
    const LRegion &bufferDamage() const noexcept;

    /**
     * @brief Gets the layout of RGB subpixels for a single pixel on a display.
     *
     * The layout of subpixels can impact the display of elements like fonts.
     */
    SubPixel subPixel() const noexcept;

    /**
     * @brief Checks if VSync control is supported for this output.
     *
     * @see enableVSync()
     *
     * @return `true` if VSync control is supported, `false` if VSync is always enabled.
     */
    bool hasVSyncControlSupport() const noexcept;

    /**
     * @brief Checks if VSync is enabled (enabled by default).
     *
     * @see enableVSync()
     *
     * @return `true` if VSync is enabled, `false` otherwise.
     */
    bool vSyncEnabled() const noexcept;

    /**
     * @brief Turns VSync on or off.
     *
     * @param enabled `true` to enable VSync, `false` to disable.
     *
     * @return `true` if VSync was successfully enabled or disabled, `false` if VSync control is not supported (see hasVSyncControlSupport()).
     */
    bool enableVSync(bool enabled) noexcept;

    /**
     * @brief Gets the refresh rate limit in Hz when VSync is disabled.
     *
     * A value less than 0 indicates the limit is disabled.\n
     * A value equal to 0 indicates the limit is double the current output mode refresh rate (the default).\n
     * Any other positive value represents the limit in Hz.
     *
     * @see setRefreshRateLimit()
     *
     * @return The refresh rate limit when VSync is disabled.
     */
    Int32 refreshRateLimit() const noexcept;

    /**
     * @brief Sets the refresh rate limit in Hz when VSync is disabled.
     *
     * A value less than 0 indicates the limit is disabled.\n
     * A value equal to 0 indicates the limit is double the current output mode refresh rate (the default).\n
     * Any other positive value represents the limit in Hz.
     *
     * @param hz The refresh rate limit in Hz when VSync is disabled.
     */
    void setRefreshRateLimit(Int32 hz) noexcept;

    /**
     * @brief Gets the size of the gamma table.
     *
     * @note This method can only be called while the output is initialized.
     *       If called when not initialized, it returns 0. If the output doesn't support gamma correction,
     *       this method also returns 0.
     *
     * @return The size of the gamma correction table.
     *
     * @see setGamma() and setGammaRequest()
     * @see LGammaTable
     */
    UInt32 gammaSize() const noexcept;

    /**
     * @brief Sets the gamma correction table for the output.
     *
     * This method allows to set the gamma correction table for the output.
     *
     * @note This method can only be called while the output is initialized.
     *       Louvre automatically sets a linear gamma table when the output is initialized.
     *
     * @param gamma A pointer to the LGammaTable with a size matching gammaSize().
     *              Passing `nullptr` restores the default table (linear).
     *
     * @return `true` if the gamma correction table was successfully set, `false` otherwise.
     */
    bool setGamma(const LGammaTable *gamma) noexcept;

    /**
     * @brief Available modes.
     *
     * This method returns a vector containing all the available output modes for the LOutput instance.
     *
     * @see setMode()
     *
     * @return A vector of pointers to LOutputMode instances representing the available modes of the output.
     */
    const std::vector<LOutputMode *> &modes() const noexcept;

    /**
     * @brief Gets the preferred mode.
     *
     * This method returns the preferred mode for the output. It is generally the mode with the highest refresh rate and resolution.
     *
     * Set by default. See setMode().
     */
    const LOutputMode *preferredMode() const noexcept;

    /**
     * @brief Gets the current mode.
     *
     * This method returns the current output mode set with setMode().
     */
    const LOutputMode *currentMode() const noexcept;

    /**
     * @brief Sets the output mode.
     *
     * Use this method to assign a mode to the output, which must be one of the available modes listed in modes().\n
     * If the mode changes and the output is already initialized the resizeGL() event is triggered.
     *
     * @note Calling this method from any of the `GL` events is not allowed, as it could potentially lead to a deadlock.
     *       In such cases, the method is simply ignored to prevent issues.
     */
    void setMode(const LOutputMode *mode) noexcept;

    /**
     * @brief Sets the output scale factor.
     *
     * Use this method to adjust the scale factor of the output. By default, outputs have a scale factor of 1.\n
     * Increasing the scale factor, such as setting it to 2, is often suitable for high-definition displays (when dpi() >= 200).
     * It's common for clients to adapt their surface scales to match the scale of the output where they are displayed.\n
     * If the scale changes and the output is already initialized, the resizeGL() event will be triggered.
     *
     * @note Starting from Louvre version 1.2, fractional scales are now supported, see the @ref Scaling "Scaling Section".
     *
     * @param scale The desired scale factor to set.
     *
     * @see See an example of its use in the default implementation of LCompositor::initialized().
     */
    void setScale(Float32 scale) noexcept;

    /**
     * @brief Retrieves the current output scale factor.
     *
     * This method returns the current scale factor assigned to the output using setScale(). The default scale factor is 1.
     *
     * If the assigned scale is fractional this value is equal to ceil(scale).
     *
     * @see fractionalScale()
     */
    Float32 scale() const noexcept;

    /**
     * @brief Gets the same scale set with setScale().
     *
     * Set to 1.f by default.
     *
     * @return The fractional scale value.
     */
    Float32 fractionalScale() const noexcept;

    /**
     * @brief Checks if the scale factor set with setScale() is fractional.
     *
     * @return `true` if the scale factor is fractional, `false` otherwise.
     */
    bool usingFractionalScale() const noexcept;

    /**
     * @brief Checks if oversampling is enabled.
     *
     * Oversampling is enabled by default when a fractional scale is set using setScale().\n
     * It is always disabled when using an integer scale. You can disable oversampling
     * using enableFractionalOversampling().
     *
     * @return `true` if oversampling is enabled, `false` otherwise.
     */
    bool fractionalOversamplingEnabled() const noexcept;

    /**
     * @brief Toggles oversampling for fractional scales.
     *
     * @note Oversampling is always turned off for integer scales.
     *       You can instantly turn oversampling on or off when using a fractional scale.
     *
     * @param enabled `true` to enable oversampling for fractional scales, `false` to disable.
     */
    void enableFractionalOversampling(bool enabled) noexcept;

    /**
     * @brief Unlocks the rendering thread.
     *
     * Calling this method unlocks the output rendering thread, triggering a subsequent paintGL() event.\n
     * Regardless of the number of repaint() calls within the same frame, paintGL() is invoked only once.\n
     * To unlock the rendering thread again, repaint() must be called within or after a paintGL() event.
     */
    void repaint() noexcept;

    /**
     * @brief Indicates that the entire output needs repainting
     *
     * This hint from the graphic backend suggests that the entire output should be repainted,
     * possibly because it doesn't support damage tracking or for other reasons.
     *
     * @note This is automatically managed by LScene.
     */
    bool needsFullRepaint() const noexcept;

    /**
     * @brief Gets the dots per inch (DPI) of the output.
     *
     * This method calculates and returns the dots per inch (DPI) of the output, considering its physical
     * dimensions and the resolution provided by its current mode.
     */
    Int32 dpi() noexcept;

    /**
     * @brief Gets the output rect.
     *
     * This method provides the position and size of the output in compositor-global coordinates (pos(), size()).
     */
    const LRect &rect() const noexcept;

    /**
     * @brief Gets the output position.
     *
     * This method retrieves the position of the output in compositor-global coordinates assigned with setPos().\n
     * It is equivalent to the position given by the rect().
     */
    const LPoint &pos() const noexcept;

    /**
     * @brief Set the position of the output.
     *
     * This method allows you to assign the position of the output in compositor-global coordinates, with the upper-left corner as the origin.\n
     * If the position changes while the output is initialized, the moveGL() event is triggered.
     *
     * @param pos The new position of the output in compositor-global coordinates.
     */
    void setPos(const LPoint &pos) noexcept;

    /**
     * @brief Gets the output size in surface units.
     *
     * This method provides the size of the output in surface units, based on its current mode and scale factor.\n
     * It is equivalent to the size given by the rect().
     */
    const LSize &size() const noexcept;

    /**
     * @brief Gets the output size in buffer units.
     *
     * This method returns the size of the output in buffer units, based on its current mode.
     */
    const LSize &sizeB() const noexcept;

    /**
     * @brief Retrieves the real destination buffer size.
     *
     * This method returns the size in pixels of the destination framebuffer during a paintGL() event.\n
     * Unlike sizeB(), the width and height are not swapped if transform() includes a 90° rotation.\n
     * Additionally, when using oversampling and a fractional scale, it returns the actual size of the
     * intermediary "oversampled" framebuffer, rather than the "fake" buffer size provided by sizeB().
     *
     * @note This method is only useful if you are not using LPainter or LScene for rendering.
     */
    const LSize &realBufferSize() const noexcept;

    /**
     * @brief Gets the physical dimensions of the output.
     *
     * This method retrieves the physical dimensions of the output in millimeters.
     *
     * @note In some cases, such as when the compositor is running inside a virtual machine, the physical size may be (0,0).
     */
    const LSize &physicalSize() const noexcept;

    /**
     * @brief Screen capture requests
     *
     * This vector contains all LScreenshotRequest s that should be handled during a paintGL() event.
     */
    const std::vector<LScreenshotRequest*> &screenshotRequests() const noexcept;

    /**
     * @brief The content type hint set with setContentType().
     */
    LContentType contentType() const noexcept;

    /**
     * @brief Sets the content type hint.
     *
     * This hint is used by some hardware displays to properly adapt to the type of content being displayed.\n
     * For example, if the @ref LContentTypeGame flag is set, a TV connected through an HDMI port may adapt
     * to reduce latency.
     *
     * The default value is @ref LContentTypeNone.
     *
     * Clients using the Content Type protocol can also specify the type of content a given LSurface is displaying.
     *
     * @see LSurface::contentType() and LSurface::contentTypeChanged().
     *
     * @param type The content type hint to be set.
     */
    void setContentType(LContentType type) noexcept;

    /**
     * @brief Gets the output name.
     *
     * This method retrieves the name of the output provided by the graphic backend, such as "HDMI-A-2."
     *
     * @note Output names are always unique, even if they belong to different GPUs.
     */
    const char *name() const noexcept;

    /**
     * @brief Gets the output model name.
     *
     * This method retrieves the model name of the output provided by the graphic backend.
     */
    const char *model() const noexcept;

    /**
     * @brief Gets the manufacturer name of the output.
     *
     * This method retrieves the manufacturer name of the output provided by the graphic backend.
     */
    const char *manufacturer() const noexcept;

    /**
     * @brief Gets the description of the output.
     *
     * This method retrieves the description of the output provided by the graphic backend.
     */
    const char *description() const noexcept;

    /**
     * @brief Gets the ID of the rendering thread.
     *
     * This method retrieves the ID of the output rendering thread.
     */
    const std::thread::id &threadId() const noexcept;

    /**
     * @name Virtual Methods
     */
///@{

    /**
     * @brief Initialize Event.
     *
     * The initializeGL() event is invoked after the output is properly initialized.\n
     *
     * @note Avoid performing painting operations here, as they won't be visible on the screen. See the paintGL() event.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp initializeGL
     */
    virtual void initializeGL();

    /**
     * @brief Paint Event.
     *
     * The paintGL() event is invoked after unlocking the rendering thread with repaint().\n
     * All rendering operations performed here will be displayed later on screen after the backend performs a page flip.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp paintGL
     */
    virtual void paintGL();

    /**
     * @brief Resize Event.
     *
     * The resizeGL() event is invoked when the output scale or mode changes.
     *
     * @note Avoid performing painting operations here, as they won't be visible on the screen. See the paintGL() event.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp resizeGL
     */
    virtual void resizeGL();

    /**
     * @brief Move Event.
     *
     * This event is triggered when the output's position changes, see setPos().\n
     *
     * @note Avoid performing painting operations here, as they won't be visible on the screen. See the paintGL() event.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp moveGL
     */
    virtual void moveGL();

    /**
     * @brief Uninitialize Event.
     *
     * The uninitializeGL() event is invoked after the output is removed from the compositor with LCompositor::removeOutput()
     * or when the output is unplugged, see LSeat::outputUnplugged().\n
     *
     * Here you should free your shaders, programs, textures, etc.
     *
     * @note Avoid performing painting operations here, as they won't be visible on the screen. See the paintGL() event.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp uninitializeGL
     */
    virtual void uninitializeGL();

    /**
     * @brief Set gamma table request.
     *
     * Clients using the [wlr gamma control](https://wayland.app/protocols/wlr-gamma-control-unstable-v1#zwlr_gamma_control_manager_v1)
     * protocol can request to set the gamma table for an output.
     *
     * @warning For security reasons, it is advisable to permit only authorized clients to perform such actions. See LCompositor::globalsFilter().
     *
     * The gamma table (when is not `nullptr`) always have a size equal to gammaSize(), hence, it is not necessary to validate that.
     *
     * @param client Pointer to the client making the request.
     * @param gamma Pointer to the LGammaTable object containing the requested gamma table or `nullptr` to restore the default table.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp setGammaRequest
     */
    virtual void setGammaRequest(LClient *client, const LGammaTable *gamma);

    /**
     * @brief Notifies a change in availableGeometry().
     *
     * This event is triggered whenever one of the exclusiveZones() changes.
     *
     * #### Default Implementation
     *
     * The default implementation adjusts and reconfigures mapped LToplevelRole surfaces
     * to prevent occlusion of the exclusive zone on each edge.
     *
     * @snippet LOutputDefault.cpp availableGeometryChanged
     */
    virtual void availableGeometryChanged();

///@}

    LPRIVATE_IMP_UNIQUE(LOutput)
};

#endif // LOUTPUT_H
