#ifndef LOUTPUT_H
#define LOUTPUT_H

#include <CZ/Louvre/LFactoryObject.h>
#include <CZ/Louvre/Backends/LBackendOutput.h>
#include <CZ/skia/core/SkSize.h>
#include <CZ/skia/core/SkRect.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/Ream/RContentType.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Core/CZTransform.h>

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
 * The rendering thread loop remains blocked until repaint() is called, which unlocks the thread and asynchronously triggers the paintGL() event.
 *
 * All painting operations must occur exclusively within a paintGL() event, as rendering elsewhere will not be visible on the screen.
 *
 * Calling repaint() multiple times will only unlock the thread once. After or during a paintGL() event, repaint() must be called again to unlock the thread once more.
 *
 * @section render_multithreading Multithreading
 *
 * In Louvre, even if there is a main thread and each initialized LOutput has its own rendering thread, no single block of user code executes in parallel (unless you create custom threads, of course).
 * For instance, all **xxxGL()** events never overlap with other events or requests being handled in separate threads.
 *
 * You may be wondering why the need to use rendering threads then?
 *
 * The answer is that calling rendering functions (in this case OpenGL functions) takes almost no time (except for **glFinish()**). Rendering commands are queued
 * and processed later together with a page flip. For instance, if your monitor has a 60 Hz refresh rate (approximately 16 ms period), a paintGL() call may take
 * 1 ms to be processed while the presentation on screen could take up to 15 ms (with V-Sync on). However, during those 15 ms, the main thread or other rendering
 * threads can continue working. This allows Louvre compositors to maintain a constant high refresh rate compared to single-threaded designs.
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
 * Given that you rotated the screen 90° clockwise, it is necessary to apply a transform that rotates the screen 90° counter-clockwise (@ref CZTransform::Rotated90).
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
 *
 * @section drm_leasing DRM Leasing
 *
 * [DRM leasing](https://wayland.app/protocols/drm-lease-v1) is a Wayland protocol and backend feature that allows clients to take control of a specific set of displays.\n
 * It is typically used by VR applications to render directly into VR headsets, skipping the compositor presentation and thereby reducing latency.
 *
 * To advertise an output as leasable, use setLeasable(). By default, Louvre disables it for all outputs except for those with isNonDesktop()
 * set to `true`, which usually indicates a VR headset. See LCompositor::initialized() and LSeat::outputPlugged().
 *
 * For each leasable output, clients can issue a leaseRequest() which can be accepted or denied, returning `true` or `false`.\n
 * Each time the output starts or stops being leased, the lease() property changes, which is notified with leaseChanged().
 *
 * The lease() property allows you to see which client is currently leasing an output and also to stop the lease by calling `lease()->finished()`.\n
 * The lease is also terminated if the compositor initializes the output or if leasable() is set to `false`.
 *
 * An output can't be initialized and leased at the same time, enabling one disables the other.
 *
 * @note Whenever the user switches to another session and the backend loses DRM master, all active leases are destroyed. Clients must request them again once the session is restored.
 */
class CZ::LOutput : public LFactoryObject
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
    ~LOutput() noexcept;

    /**
     * @brief Retrieves a unique identifier for the output.
     *
     * This identifier is guaranteed to be unique for each output.
     *
     * When using the DRM backend, it corresponds to the underlying DRM connector ID.
     */
    UInt32 id() const noexcept { return m_backend->id(); }

    /**
     * @brief Retrieves the age of the current image.
     *
     * This method returns the age of the buffer as specified in the
     * [EGL_EXT_buffer_age](https://registry.khronos.org/EGL/extensions/EXT/EGL_EXT_buffer_age.txt)
     * extension specification.
     */
    UInt32 imageAge() const noexcept;

    /**
     * @brief Returns the index of the backendImage().
     *
     * @note For damage tracking, use imageAge() instead.
     */
    UInt32 imageIndex() const noexcept { return m_backend->imageIndex(); };

    /**
     * @brief Returns the current target image for the paint event.
     *
     * This is either the backend image or the oversampled image, depending on whether
     * oversampling is enabled. Internally, it delegates to either backendImage() or osImage().
     */
    std::shared_ptr<RImage> image() const noexcept;

    /**
     * @brief Returns the backend-provided image used for painting.
     *
     * This is the default image used when oversampling is disabled.
     */
    std::shared_ptr<RImage> backendImage() const noexcept;

    /**
     * @brief Returns the oversampled image used for painting when oversampling is enabled.
     */
    std::shared_ptr<RImage> osImage() const noexcept;

    /**
     * @brief Backend images.
     */
    const std::vector<std::shared_ptr<RImage>> &images() const noexcept { return m_backend->images(); }

    /**
     * @brief Gets the layout of RGB subpixels for a single pixel on a display.
     *
     * The layout of subpixels can impact the display of elements like fonts.
     */
    RSubpixel subpixel() const noexcept { return backend()->subpixel(); }

    /**
     * @brief Checks if VSync can be disabled for this output.
     */
    bool canDisableVSync() const noexcept { return backend()->canDisableVSync(); }

    /**
     * @brief Checks if VSync is enabled (enabled by default).
     */
    bool isVSyncEnabled() const noexcept { return backend()->isVSyncEnabled(); }

    /**
     * @brief Toggles VSync.
     */
    bool enableVSync(bool enabled) noexcept { return backend()->enableVSync(enabled); }

    /**
     * @brief Gets the size of the gamma table.
     *
     * @return The size of the gamma correction table or 0 if unsupported or the output is uninitialized.
     */
    size_t gammaSize() const noexcept { return backend()->gammaSize(); }

    /**
     * @brief Sets the gamma LUT for the output.
     *
     * @note This method can only be called while the output is initialized.
     *       Louvre automatically sets a linear gamma table when the output is initialized.
     *
     * @param gamma A pointer to the LGammaTable with a size matching gammaSize().
     *              Passing `nullptr` restores the default table (linear).
     *
     * @return `true` if the gamma correction table was successfully set, `false` otherwise.
     */
    bool setGammaLUT(std::shared_ptr<const RGammaLUT> gamma) noexcept;
    std::shared_ptr<const RGammaLUT> gammaLUT() const noexcept { return backend()->gammaLUT(); }

    /**
     * @brief Available modes (refresh rate and resolution).
     *
     * @see setMode()
     */
    const std::vector<std::shared_ptr<LOutputMode>> &modes() const noexcept { return m_backend->modes(); }

    /**
     * @brief Gets the preferred mode.
     *
     * This is generally the mode with the highest refresh rate and resolution.
     *
     * Set by default. See setMode().
     */
    std::shared_ptr<LOutputMode> preferredMode() const noexcept { return m_backend->preferredMode(); }

    /**
     * @brief Gets the current mode.
     */
    std::shared_ptr<LOutputMode> currentMode() const noexcept { return m_backend->currentMode(); }

    /**
     * @brief Sets the output mode.
     *
     * If the mode changes and the output is already initialized the resizeGL() event is triggered.
     */
    int setMode(std::shared_ptr<LOutputMode> mode) noexcept;

    /**
     * @brief Gets the current state of the LOutput.
     */
    State state() const noexcept;

    /**
     * @brief The device this output belongs.
     */
    RDevice *device() const noexcept { return m_backend->device(); };

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
    const SkIRect &availableGeometry() const noexcept;

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
     * @brief Gets the current transform.
     *
     * The default value is @ref CZTransform::Normal.
     */
    CZTransform transform() const noexcept;

    /**
     * @brief Sets the framebuffer transform.
     *
     * This method sets the output transform, allowing you to adjust the orientation/flipping of the output.
     * If the specified transform includes a 90 or 270-degree rotation, the width and height of the output are swapped accordingly.
     */
    void setTransform(CZTransform transform) noexcept;

    /**
     * @brief Sets the output scale factor.
     *
     * LCompositor::initialized() assigns the default value based on the dpi() e.g. setting it to 2 is often suitable for high-definition displays (when dpi() >= 200).
     *
     * Supported range: [0.25, inf]
     */
    void setScale(Float32 scale) noexcept;

    /**
     * @brief Retrieves the current output scale factor.
     *
     * @note If the assigned scale is fractional this value is equal to ceil(scale).
     *
     * @see fractionalScale()
     */
    Float32 scale() const noexcept;

    /**
     * @brief Gets the scale factor set with setScale().
     */
    Float32 fractionalScale() const noexcept;

    /**
     * @brief Checks if the scale factor set with setScale() is fractional.
     */
    bool usingFractionalScale() const noexcept;

    /**
     * @brief Checks if oversampling is enabled.
     *
     * Oversampling is enabled by default, however, it is never used when using an integer scale.
     *
     * When enabled with a fractional scale, rendering is performed into a pixel-perfect intermediate
     * image, which Louvre then blits into the real screen image (see osImage()).
     * This helps eliminate aliasing artifacts but may generate blurry images and affect performance.
     *
     * To disable it, call enableOversampling(false).
     */
    bool oversamplingEnabled() const noexcept;

    /**
     * @brief Toggles oversampling.
     */
    void enableOversampling(bool enabled) noexcept;

    /**
     * @brief Unlocks the rendering thread.
     *
     * Calling this method unlocks the output rendering thread, triggering a subsequent paintGL() event.
     * Regardless of the number of repaint() calls within the same frame, paintGL() is invoked only once.
     * To unlock the rendering thread again, repaint() must be called within or after a paintGL() event.
     */
    void repaint() noexcept;

    /**
     * @brief Indicates that the entire output needs repainting.
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
    const SkIRect &rect() const noexcept;

    /**
     * @brief Gets the output position.
     *
     * This method retrieves the position of the output in compositor-global coordinates assigned with setPos().
     * It is equivalent to the position given by the rect().
     */
    SkIPoint pos() const noexcept;

    /**
     * @brief Set the position of the output.
     *
     * This method allows you to assign the position of the output in compositor-global coordinates, with the upper-left corner as the origin.
     * If the position changes while the output is initialized, the moveGL() event is triggered.
     *
     * @param pos The new position of the output in compositor-global coordinates.
     */
    void setPos(SkIPoint pos) noexcept;

    /**
     * @brief Gets the output size in surface units.
     *
     * This method provides the size of the output in surface units, based on its current mode and scale factor.
     * It is equivalent to the size given by the rect().
     */
    SkISize size() const noexcept;

    /**
     * @brief Gets the output size in buffer units.
     *
     * This method returns the size of the output in buffer units, based on its current mode.
     */
    SkISize sizeB() const noexcept;

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
    SkISize realBufferSize() const noexcept;

    /**
     * @brief Gets the physical dimensions of the output.
     *
     * This method retrieves the physical dimensions of the output in millimeters.
     *
     * @note In some cases, such as when the compositor is running inside a virtual machine, the physical size may be (0,0).
     */
    SkISize mmSize() const noexcept { return m_backend->mmSize(); }

    /**
     * @brief The content type hint set with setContentType().
     */
    RContentType contentType() const noexcept { return m_backend->contentType(); }

    /**
     * @brief Sets the content type hint.
     *
     * This hint is used by some hardware displays to properly adapt to the type of content being displayed.\n
     * For example, if the @ref RContentType::Game flag is set, a TV connected through an HDMI port may adapt
     * to reduce latency.
     *
     * The default value is @ref LContentTypeNone.
     *
     * Clients using the Content Type Hint protocol can also specify the type of content a given LSurface is displaying.
     *
     * @see LSurface::contentType() and LSurface::contentTypeChanged().
     *
     * @param type The content type hint to be set.
     */
    void setContentType(RContentType type) noexcept { m_backend->setContentType(type); }

    /***************************** DRM LEASING *****************************/

    /**
     * @brief Determines if the output is not intended for desktop usage.
     *
     * E.g. Outputs like VR headsets set this property to `true`.
     *
     * Such outputs are typically intended to be leased by clients and not used by the compositor.
     *
     * The default implementation of LCompositor::initialized() marks outputs with this property as leasable. See setLeasable().
     *
     * @return `true` if the output is not meant for desktop usage, `false` otherwise.
     */
    bool isNonDesktop() const noexcept { return m_backend->isNonDesktop(); }

    /**
     * @brief Advertises the output as leasable.
     *
     * When set to `true`, clients using the DRM Lease protocol will be able to trigger leaseRequest().
     *
     * @note Setting it to `false` will automatically revoke any existing lease.
     */
    void setLeasable(bool leasable) noexcept;

    /**
     * @brief Checks if the output is advertised as leasable.
     *
     * @see setLeasable() and leaseChanged()
     *
     * @return `true` if leasable, `false` otherwise. `false` by default.
     */
    bool leasable() noexcept;

    /**
     * @brief Current lease created after an accepted leaseRequest().
     *
     * `#include <CZ/Louvre/Protocols/DRMLease/RDRMLease.h>`
     *
     * Use this property to determine which client is leasing the output via `lease()->client()` or to revoke it with `lease()->finished()`.
     *
     * Once revoked, this property is set to `nullptr`.
     *
     * @return `nullptr` if the output is not being leased.
     */
    Protocols::DRMLease::RDRMLease *lease() const noexcept;

    /**
     * @brief Handles client requests to lease the output.
     *
     * This request is only triggered if the output was previously advertised as leasable with setLeasable().
     *
     * Accepting the request does not guarantee that the lease() property will be immediately set.
     * The graphics backend may fail to create it or the client may issue this request for additional outputs before it is created.
     *
     * If accepted, any other client will stop leasing it. Additionally, if it is being used by the compositor, it will be uninitialized.
     *
     * @return `true` to allow the client, `false` to deny.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp leaseRequest
     */
    virtual bool leaseRequest(LClient* client);

    /**
     * @brief Notifies a change in the lease() property.
     *
     * This event is triggered whenever a client starts or stops leasing the output.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp leaseChanged
     */
    virtual void leaseChanged();

    /**
     * @brief Gets the output name.
     *
     * This method retrieves the name of the output provided by the graphic backend, such as "HDMI-A-2."
     *
     * @note Output names are always unique, even if they belong to different GPUs.
     */
    const std::string &name() const noexcept { return m_backend->name(); }

    /**
     * @brief Gets the output model name.
     *
     * This method retrieves the model name of the output provided by the graphic backend.
     */
    const std::string &model() const noexcept { return m_backend->model(); }

    /**
     * @brief Gets the manufacturer name of the output.
     *
     * This method retrieves the manufacturer name of the output provided by the graphic backend.
     */
    const std::string &manufacturer() const noexcept { return m_backend->make(); }

    /**
     * @brief Gets the description of the output.
     *
     * This method retrieves the description of the output provided by the graphic backend.
     */
    const std::string &description() const noexcept { return m_backend->desc(); }

    /**
     * @brief Retrieves the serial number of the output.
     *
     * The serial number can be used to uniquely identify an output even after reboots.
     *
     * @return `nullptr` if the backend doesn't provide it.
     */
    const std::string &serialNumber() const noexcept { return m_backend->serial(); }

    /**
     * @brief Gets the ID of the rendering thread.
     *
     * This method retrieves the ID of the output rendering thread.
     */
    const std::thread::id &threadId() const noexcept;

    LBackendOutput *backend() const noexcept { return m_backend.get(); }

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

    virtual void presentationEvent(const CZPresentationEvent &e) noexcept;

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
    virtual void setGammaRequest(LClient *client, std::shared_ptr<const RGammaLUT> gamma);

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

    /**
     * @brief Temporarily disables repaint calls for this output.
     *
     * An output locks its rendering thread until repaint() is called. However, many objects and implementations
     * within the compositor may automatically trigger repaints to reflect changes, making it difficult to
     * prevent unwanted repaints.
     *
     * This method intercepts repaint() calls and preserves the last rendered frame. When `false` is returned,
     * the repaint() calls are ignored, and paintGL() is not triggered.
     *
     * @note During initialization, uninitialization, or mode changes, the rendering thread is forcefully
     *       unlocked and the filter is ignored to prevent deadlocks.
     *
     * #### Default Implementation
     *
     * The default implementation accepts all repaint() calls.
     *
     * @snippet LOutputDefault.cpp repaintFilter
     *
     * @return `true` if the repaint should proceed, `false` otherwise.
     */
    virtual bool repaintFilter();

    /**
     * @brief Damaged region during a paintGL event.
     *
     * Indicates the area updated during paintGL(), helping the graphics backend reduce
     * the amount of pixels to copy (useful with the raster backend, oversampling, or hybrid GPU setups).
     *
     * Defined in output-local coordinates. Reset to full damage ((0, 0), LOutput::size()) before each paintGL() call.
     */
    SkRegion damage;

    bool event(const CZEvent &e) noexcept override;

///@}
    LPRIVATE_IMP_UNIQUE(LOutput)
    std::shared_ptr<LBackendOutput> m_backend;
};

#endif // LOUTPUT_H
