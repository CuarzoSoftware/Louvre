#ifndef LOUTPUT_H
#define LOUTPUT_H

#include <LObject.h>
#include <LSize.h>
#include <LRect.h>
#include <LRegion.h>
#include <LFramebuffer.h>

#include <thread>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <sys/eventfd.h>
#include <sys/poll.h>

/**
 * @brief A display rendering interface.
 *
 * The LOutput class is responsible for rendering content to a display. It is typically associated with a computer screen, but could also
 * represent a window within an X11 or Wayland desktop, depending on the selected graphic backend.
 *
 * <center><IMG height="250px" SRC="https://lh3.googleusercontent.com/4lV1LTHBmO-eFywBrL4UhYIRcQbV5bjGB_17FdWFCzjGvnklxwBnXz5hQKOrkRCOegsn6PjnYZNCWk1SjFjwh9t8olEzr3Uwzd3saEt8EKRbbqX0n1f5R7q6r6V9u1t0PUk7BB0teA"></center>
 *
 * @section Access
 *
 * The graphic backend is responsible for creating each LOutput making a request to the compositor through LCompositor::createOutputRequest().\n
 * You can override that virtual constructor to use your own LOutput subclasses.\n
 * The LSeat class grants access to all outputs created by the graphic backend through the LSeat::outputs() list and it also notifies you of
 * hotplugging events (e.g. when connecting or disconnecting a display through an HDMI port).
 *
 * @see LSeat::outputPlugged() and LSeat::outputUnplugged()
 *
 * @section Initialization
 *
 * By default, outputs are inactive and cannot be rendered on. To activate an output, use the LCompositor::addOutput() method.
 * This initializes its rendering thread, graphic context, and triggers the initializeGL() event.
 * Subsequently, whenever the repaint() method is called, the rendering thread unblocks, and the paintGL() event is invoked,
 * allowing you perform painting operations.
 *
 * @note Louvre by default initializes all available outputs once the compositor is started within the LCompositor::initialized() event.
 *
 * @section Rendering
 *
 * Painting operations must exclusively take place within a paintGL() event, as rendering elsewhere won't be visible on the screen.\n
 * When you call repaint(), Louvre unlocks the output rendering thread and invokes paintGL() just once, regardless of the number of
 * repaint() calls during a frame.\n
 * To unlock the rendering thread again, you must call repaint() within or after the last paintGL() event.\n
 * The library is in charge of triggering the initializeGL(), moveGL(), resizeGL(), paintGL(), and uninitializeGL() events,
 * so you must avoid calling them directly.
 *
 * @section Context
 *
 * Each output has its own OpenGL context and its own instance of LPainter, which can be accessed with the painter() method.\n
 * You can use the functions provided by LPainter to render colored rects or textures, or use the native OpenGL functions and your
 * own shaders/programs if you wish.
 *
 * @note Consider leveraging the LScene and LView classes for rendering, as they automatically calculate and repaint only the
 *       portions of an output that require updates. This can significantly enhance the performance of your compositor.
 *
 * @section Modes
 *
 * Each LOutput can have multiple modes. An LOutputMode contains information about the resolution and refresh rate that the output can operate at.\n 
 * You can access the modes of an output with modes() and set the desired one with setMode().\n
 * Outputs by default use the preferredMode(), which typically has the highest refresh rate and resolution.\n
 * If you change an output's mode or scale while it's initialized, the resizeGL() event is triggered.
 *
 * @section Arrangement
 *
 * Outputs, like surfaces, have a position and dimensions that allow them to be logically organized in a similar way to how a system settings panel does.\n
 * You can adjust the position of an output using the setPos() method, which will, in turn, trigger the moveGL() event when the position changes.
 *
 * <center><IMG height="350px" SRC="https://lh3.googleusercontent.com/VOWUX4iiqYMF_bIrBP3xMyaiydv_e_ZKznCIJlRLaEA0CtBLMuU4h41R3D4Xm-7krk8jFGZrQGb_SS7hlIFUY9E5dVbQqs0Q3NIBXvRFrGs_cukqOmbCv1ExN9fG3BDdj4Yz45xIkQ=w2400"></center>
 *
 * @note To enable LCursor to transition across different LOutputs, ensure that the outputs are closely arranged side by side.
 *
 * @section Scaling
 *
 * Many screens nowadays are HiDPI, so it is commonly required to apply a scaling factor to prevent content from appearing tiny on the screen.
 * By default, all screens in Louvre have a scaling factor of 1, meaning no scaling is applied. To assign a scale to a screen, you can use setScale(),
 * which modifies the size returned by size().
 *
 * In Louvre, you typically work with two coordinate systems: buffer coordinates and surface coordinates.
 *
 * In buffer coordinates, the scale is not taken into account, and the dimensions always have maximum granularity.
 * For example, if a screen has a resolution of 2000x1000px, its size in buffer coordinates would be the same: 2000x1000px, which is returned by sizeB().
 * However, the global coordinate space of the compositor uses surface coordinates, which is equal to the size in buffer coordinates divided by the applied scale.
 * Therefore, if a factor of 2 is used, the size in surface coordinates would be 1000x500, which is returned by size().
 *
 * When arranging displays, both the position and size of the outputs should be considered in surface coordinates.
 *
 * Let's look at an example to make these concepts clearer:
 *
 * Let's assume you have two displays, one with a resolution of 1000x500px and another with 2000x1000px (double the resolution), but both with a physical size of 22''.
 * This means that the space occupied by 1px on the blue screen accommodates 4px on the pink screen. If you were to see it in person, side by side they would look like:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV87QBEaAMVfuF92jfv0HRQrEgc0PMKgx9N29Wb6dDbF9cHLpCr7qSDUowUFnBXFHJxg4F9c7v7EcxxTSnpSzkEqLvCB7CxlnUYJmG1JsNspSHRq3zZE=w2400"></center>
 *
 * If you assign the same scaling factor of 1 to both screens, their sizes in surface coordinates would be the same as in buffer coordinates.
 * Therefore, the global coordinate space of the compositor would look like this.
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV86CZvhAIc-yONr5BQe6ABGxfcuSVajo1jobAWL5C6tMdJO8BXjsbMbZk7r7rEZ9-ZsYmr3LsIdBSKkb3npEYoCIG1OD0iZKUoCDFVoqMG4avt06Vhs=w2400"></center>
 *
 * And if you were to see your screens in real life, it would look like the following. In the pink screen, everything would appear tiny, half the size of the blue screen.
 * And as you can see, if you were to drag an application window from one screen to another, it would look somewhat odd.
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV84F3o3dewpcSRttqMHPjKO2YYUqSJ299tj760KcT6KRd0s7uf_bXtKQfwy5CFeXqPoxynuu2UmtkEOodg1l7DjZHoXohjXdpGIth9S50mKGdsHqfPw=w2400"></center>
 *
 * Now, let's imagine that you assign a scaling factor of 2 to the pink screen. In this case, the global coordinate space of the compositor would look as follows:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV86j52vqCYAxcm94frzQHzeI47idYlb-tggnlbwcVsZZwXIkr9M4tyhKPMDLfmtjOYqHWS9jnCiXojuusl8EKyv3OIn8KQX6biHr0hQeLxH7m04VaTc=w2400"></center>
 *
 * And therefore, if you were to see it in real life now, it would appear consistently.
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV87mN6ufD6PY9Rt8fjl3B84ii627PdOYwJx9vmeA72EeT_cpr8Q01IdoiBknoPI6fonVJWRCW34VZNKVKCsNIERY70Gvqkl7eH0R2vr3gBrEtuMNps0=w2400"></center>
 *
 * Therefore, in summary, the size of a screen in surface coordinates is determined by dividing its buffer size by the applied scale.
 *
 * ## Fractional Scaling
 *
 * Louvre also supports fractional scaling, albeit with some differences compared to integer scaling. For instance, if you assign a scale of 1.5 using setScale()
 * to the pink screen, the resulting applied scale (obtained with scale()) will be ceil(1.5) = 2. However, the buffer dimensions of the screen will simulate
 * being 2/1.5 times larger than its current mode (rounded), resulting in 2668 x 1334 for this case. Consequently, its size in surface coordinates would be
 * 2668 x 1334 divided by 2.
 * As a result, the compositor coordinate space would appear as follows:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV86R-d2uzDybJ-FLWL5PzbShg33f2_OJX9mQME8DkVI5y6O3eDT18UBRgwY8UScaApeoqU4ePq3BwbDNmW5Z-eW0dukpvdQfmarUbOVEbN_GmYU3MqE=w2400"></center>
 *
 * This creates the illusion of rendering on a larger screen, the result of which is actually scaled to the real size, allowing for the desired scaling effect.
 * Rendering using fractional scales, however, can introduce undesired visual effects like aliasing, especially noticeable when moving elements with textures containing fine details.
 * For this reason, Louvre offers the option to render using oversampling, where all the screen content is rendered in a larger buffer, and then that rendered buffer is scaled down to screen framebuffer.
 * This method almost completely eliminates aliasing but has the disadvantage of consuming more computational power, potentially decreasing performance.
 * Without oversampling the content directly rendered on the screen, making it efficient but retaining aliasing artifacts.
 * Louvre allows you to toggle oversampling on and off instantly at any time using enableFractionalOversampling(). For example, you could enable it when displaying a desktop with flating windows and disable it when displaying a fullscreen window.
 *
 * @note Oversampling is not required and is always deactivated when using non-fractional scales. Therefore, as a recommendation, if your monitor supports multiple modes with various resolutions,
 *       it is preferable to select one of those modes instead of using fractional scaling.
 *
 * Clients supporting the fractional scaling protocol are instructed to scale their buffers directly to the fractional scale.
 * On the other hand, clients lacking support for the protocol are advised to use ceil(fractional scale), ensuring a consistently high-detail appearance.
 *
 * @section Transforms
 *
 * Louvre also supports applying transforms to outputs with setTransform().
 * Let's imagine that you physically rotate the pink monitor 90° clockwise while maintaining the same normal transform on both.
 * What you would see is the following:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV85bV4h-OcbW1DozgUCQp4PbMoWOd2E2MNRQveEhw5r2p_mjnVRLbziTJ-egMu9D0eSIbMyLNmXECbupfa1gmQZMAidIS1pT5ahbeoL6hcsa6O3QlYc=w2400"></center>
 *
 * Given that you rotated the screen 90° clockwise, it is necessary to apply a transform that rotates the screen 90° counter-clockwise (Louvre::LFramebuffer::Rotated90).
 * If you apply this to the pink screen, then you would see the following:
 *
 * <center><IMG SRC="https://lh3.googleusercontent.com/pw/ABLVV84Tm3VGwuNHqbAumAVrzZVgBtKSOo24y8PDk5Y47hIlLmO7uSzIMLeE0f9y-5DOQh1nOI9-qz48c_fnlgkM3CILC-GFB2qttGaE671Xke_jcF6DlVY=w2400"></center>
 *
 * Note that when applying a transformation containing a 90° rotation to an output, its width becomes its height, and its height becomes its width.
 * The global coordinate space of the compositor is structured in such a way that you can continue rendering in the same manner
 * as if the screens were in their normal transform state. Therefore, there's no need to worry about rotating or flipping the elements you draw.
 *
 * @section Uninitialization
 *
 * If you no longer wish to use an output, call the LCompositor::removeOutput() method to
 * remove it from the compositor. This will only uninitialize it, making it possible to re-initialize it later.\n
 * An LOutput is no longer available when its virtual destructor is invoked (LCompositor::destroyOutputRequest()).
 */
class Louvre::LOutput : public LObject
{
public:

    /**
     * @brief Enumeration of possible states for an LOutput.
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
     * @brief Enumeration describing the subpixel geometry.
     *
     * This enumeration provides information about how the physical pixels on an output are laid out.
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

    // Default is enabled
    bool fractionalOversamplingEnabled() const;
    bool usingFractionalScale() const;
    void enableFractionalOversampling(bool enabled);
    Float32 fractionalScale() const;
    SubPixel subPixel() const;
    bool hasVSyncControlSupport() const;
    bool vSyncEnabled() const;
    bool enableVSync(bool enabled);
    Int32 refreshRateLimit() const;
    void setRefreshRateLimit(Int32 hz);

    /**
     * @brief Constructor of the LOutput class.
     */
    LOutput();

    /**
     * @brief Destructor of the LOutput class.
     *
     * Invoked internally by the library after LCompositor::destroyOutputRequest() is called.
     */
    virtual ~LOutput();

    /// @cond OMIT
    LOutput(const LOutput&) = delete;
    LOutput& operator= (const LOutput&) = delete;
    /// @endcond

    /**
     * @brief Get the current state of the LOutput.
     *
     * This method returns the current state of the output.
     */
    State state() const;

    /**
     * @brief Return a pointer to the associated framebuffer.
     *
     * @return A pointer to the LFramebuffer instance associated with the output.
     */
    LFramebuffer *framebuffer() const;

    /**
     * @brief Get the framebuffer transformation.
     *
     * This method returns the current framebuffer transformation applied with setTransform().
     * The default value is LFramebuffer::Normal.
     *
     * @return The framebuffer transformation.
     */
    LFramebuffer::Transform transform() const;

    /**
     * @brief Set the framebuffer transformation.
     *
     * This method sets the transformation for the framebuffer, allowing you to adjust the orientation of the output.
     * If the specified transformation includes a 90 or 270-degree rotation, the width and height of the output are swapped accordingly.
     *
     * @param transform The framebuffer transformation to apply.
     */
    void setTransform(LFramebuffer::Transform transform);

    /**
     * @brief Return the index of the current buffer.
     *
     * Compositors commonly employ double or triple buffering to ensure smooth graphics rendering. This involves rendering to one buffer while displaying another, reducing visual artifacts like glitches and tearing.
     *
     * @image html https://lh3.googleusercontent.com/2ousoWwxnVGvFX5bT6ual2G8UUbhUOJ21mK1UQmthPNM-7XfracRlL5GCYBQTzt4Os28eKO_FzC6BS-rasiNngvTMI9lEdET0ItKrI2wK_9IwSDaF-hNGkTMI6gVlL0m4ENDJYbckw
     *
     * @return The current buffer index. Alternates between [0], [0, 1] or [0, 1, 2] depending on the graphic backend configuration.
     *
     * @see [Graphic Backend Configuration](md_md__environment.html#graphic)
     */
    Int32 currentBuffer() const;

    /**
     * @brief Return the count of available buffers.
     *
     * This method returns the number of buffers used by the output. It can be 1, 2 or 3 depending on the graphic backend configuration.
     *
     * @see [Graphic Backend Configuration](md_md__environment.html#graphic)
     */
    UInt32 buffersCount() const;

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
    LTexture *bufferTexture(UInt32 bufferIndex);

    /**
     * @brief Check if the output supports buffer damage tracking.
     *
     * Some graphic backends/hardware can benefit from knowing which regions of the framebuffer have changed within a paintGL() event.
     * This method indicates whether buffer damage support is available.
     *
     * @see setBufferDamage()
     *
     * @return `true` if the graphical backend supports buffer damage tracking, `false` otherwise.
     */
    bool hasBufferDamageSupport() const;

    /**
     * @brief Specify the damaged region of the framebuffer.
     *
     * This method is used to designate the region of the framebuffer that has been affected in the most recent paintGL() event.
     * It should be invoked after completing all painting operations during a paintGL() event.
     * The damage region is automatically cleared just before the subsequent paintGL() event.
     *
     * @note Although calling this method is not mandatory, it can significantly enhance performance, especially on certain graphic backends/hardware
     *       or when the output scale is fractional and oversampling is enabled. If never invoked, the entire output is considered damaged.
     *
     * When using LScene for rendering, damage calculation is handled automatically, and calling this method should be avoided.
     *
     * @param damage The damaged region of the framebuffer in surface coordinates. Providing an empty region indicates no damage, while passing `nullptr` implies the entire output is damaged.
     */
    void setBufferDamage(const LRegion *damage);

    /**
     * @brief List of available modes.
     *
     * This method returns a list containing all the available output modes for the LOutput instance.
     *
     * @return A list of pointers to LOutputMode instances representing the available modes of the output.
     */
    const std::list<LOutputMode *> &modes() const;

    /**
     * @brief Get the preferred mode.
     *
     * This method returns the preferred mode for the output. It is generally the mode with the highest refresh rate and resolution.
     */
    const LOutputMode *preferredMode() const;

    /**
     * @brief Get the current mode.
     *
     * This method returns the current output mode.
     */
    const LOutputMode *currentMode() const;

    /**
     * @brief Set the output mode.
     *
     * Use this method to assign a mode to the output, which must be one of the available modes listed in modes().
     * If the mode changes and the output is already initialized the resizeGL() event is triggered.
     *
     * @note Calling this method from any of the `GL` events is not allowed, as it could potentially lead to a deadlock.
     *       In such cases, the method is simply ignored to prevent issues.
     */
    void setMode(const LOutputMode *mode);

    /**
     * @brief Set the output scale factor.
     *
     * Use this method to adjust the scale factor of the output. By default, outputs have a scale factor of 1.
     * Increasing the scale factor, such as setting it to 2, is often suitable for high-definition displays (when dpi() >= 200).
     * It's common for clients to adapt their surface scales to match the scale of the output where they are displayed.
     * If the scale changes and the output is already initialized, the resizeGL() event will be triggered.
     *
     * @param scale The desired scale factor to set.
     *
     * @see See an example of its use in the default implementation of LCompositor::initialized().
     */
    void setScale(Float32 scale);

    /**
     * @brief Retrieve the current output scale factor.
     *
     * This method returns the current scale factor assigned to the output using setScale(). The default scale factor is 1.
     */
    Float32 scale() const;

    /**
     * @brief Schedule the next rendering frame.
     *
     * Calling this method unlocks the output rendering thread, triggering a subsequent paintGL() event.
     * Regardless of the number of repaint() calls within the same frame, paintGL() is invoked only once.\n
     * To unlock the rendering thread again, repaint() must be called within or after a paintGL() event.
     */
    void repaint();

    /**
     * @brief Get the dots per inch (DPI) of the output.
     *
     * This method calculates and returns the dots per inch (DPI) of the output, considering its physical
     * dimensions and the resolution provided by its current mode.
     */
    Int32 dpi();

    /**
     * @brief Get the physical dimensions of the output.
     *
     * This method retrieves the physical dimensions of the output in millimeters.
     *
     * @note In some cases, such as when the compositor is running inside a virtual machine, the physical size may be (0,0).
     */
    const LSize &physicalSize() const;

    /**
     * @brief Get the output size in compositor coordinates.
     *
     * This method provides the size of the output in surface coordinates, based on its current mode and scale.
     * It is equivalent to the size given by the rect() method.
     */
    const LSize &size() const;

    /**
     * @brief Get the output size in buffer coordinates.
     *
     * This method returns the size of the output in buffer coordinates, based on its current mode.
     *
     * @note Since the dimensions provided by this method are in buffer coordinates, they are not affected by the output scale.
     */
    const LSize &sizeB() const;

    /**
     * @brief Get the output rect.
     *
     * This method provides the position and size of the output in surface coordinates (pos(), size()).
     */
    const LRect &rect() const;

    /**
     * @brief Get the output position in surface coordinates.
     *
     * This method retrieves the position of the output in surface coordinates assigned with setPos().
     * It is equivalent to the position given by the rect() method.
     */
    const LPoint &pos() const;

    /**
     * @brief Set the position of the output.
     *
     * This method allows you to assign the position of the output in surface coordinates, with the upper-left corner as the origin.
     * If the position changes while the output is initialized, it will trigger the moveGL() event.
     *
     * @param pos The new position of the output.
     */
    void setPos(const LPoint &pos);

    /**
     * @brief Get the EGLDisplay handle.
     *
     * This method retrieves the EGLDisplay of the output created by the graphic backend.
     */
    EGLDisplay eglDisplay();

    /**
     * @brief Get the output name.
     *
     * This method retrieves the name of the output provided by the graphic backend, such as "HDMI-A-2."
     *
     * @note Output names are always unique, even if they belong to different GPUs.
     */
    const char *name() const;

    /**
     * @brief Get the output model name.
     *
     * This method retrieves the model name of the output provided by the graphic backend.
     */
    const char *model() const;

    /**
     * @brief Get the manufacturer name of the output.
     *
     * This method retrieves the manufacturer name of the output provided by the graphic backend.
     */
    const char *manufacturer() const;

    /**
     * @brief Get the description of the output.
     *
     * This method retrieves the description of the output provided by the graphic backend.
     */
    const char *description() const;

    /**
     * @brief Get access to the associated LPainter.
     *
     * This method provides access to the LPainter associated with this output.
     */
    LPainter *painter() const;

    /**
     * @brief Get the ID of the rendering thread.
     *
     * This method retrieves the ID of the output rendering thread.
     */
    const std::thread::id &threadId() const;

    /**
     * @name Virtual Methods
     */
///@{

    /**
     * @brief Initialize Event.
     *
     * The initializeGL() event is invoked by the library after the output is properly initialized.\n
     * You can override this method to initialize your shaders, programs, textures, etc.
     *
     * The default implementation assigns the white color to clear the screen.
     *
     * @note Avoid performing painting operations here, as they won't be visible on the screen. Instead, perform painting tasks in the paintGL() event.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp initializeGL
     */
    virtual void initializeGL();

    /**
     * @brief Paint Event.
     *
     * The paintGL() event is invoked by the library after calling the repaint() method.\n
     * Override this method to perform your painting operations.
     *
     * The default implementation clears the screen, draws the surfaces following the order of the LCompositor::surfaces() list,
     * draws the cursor if the graphic backend does not support cursor composition via hardware, and
     * finally draws the icon of a drag & drop session if there was one.
     *
     * @note The default implementation provides a basic rendering method that is quite inefficient since it redraws the entire screen every frame.
     *       Check the code of the [louvre-weston-clone](md_md__examples.html#weston) example compositor to see how to render efficiently with LPainter
     *       or consider using the LScene and LView rendering system.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp paintGL
     */
    virtual void paintGL();

    /**
     * @brief Resize Event.
     *
     * The resizeGL() event is invoked by the library when the output scale or mode changes.\n
     * Override this method to readjust your graphical interfaces.
     *
     * @note Avoid performing any painting operations here, as they won't be visible on the screen.
     *       Instead, perform painting tasks in the paintGL() event.
     *
     * The default implementation simply schedules a new rendering frame with repaint().
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp resizeGL
     */
    virtual void resizeGL();

    /**
     * @brief Move Event.
     *
     * This event is triggered when the output's position changes, such as when the setPos() method is called.\n
     * Override this method to readjust your graphical interfaces.
     *
     * @note Avoid performing any painting operations here, as they won't be visible on the screen.
     *       Instead, perform painting tasks in the paintGL() event.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp moveGL
     */
    virtual void moveGL();

    /**
     * @brief Uninitialize Event.
     *
     * The uninitializeGL() event is invoked by the library after the output is removed from the compositor with LCompositor::removeOutput().\n
     * This event is also automatically invoked when the output is unplugged, check the LSeat::outputUnplugged() event.\n
     * Override this method to free your shaders, programs, textures, etc.
     *
     * @note Avoid performing any painting operations here, as they won't be visible on the screen.
     *       Instead, perform painting tasks in the paintGL() event.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp uninitializeGL
     */
    virtual void uninitializeGL();

///@}

    LPRIVATE_IMP_UNIQUE(LOutput)
};

#endif // LOUTPUT_H
