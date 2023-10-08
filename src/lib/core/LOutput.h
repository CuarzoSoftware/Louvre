#ifndef LOUTPUT_H
#define LOUTPUT_H

#include <LObject.h>
#include <LSize.h>
#include <LRect.h>
#include <LRegion.h>

#include <thread>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <sys/eventfd.h>
#include <sys/poll.h>

/**
 * @brief A display used for rendering.
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
 * @section Modes
 *
 * Each LOutput can have multiple modes. An LOutputMode contains information about the resolution and refresh rate that the output can operate at.\n 
 * You can access the modes of an output with modes() and set the desired one with setMode().\n
 * Outputs by default use the preferredMode(), which typically has the highest refresh rate and resolution.\n
 * If you change an output's mode or scale while it's initialized, the resizeGL() event is triggered.
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
 * @section Layout
 *
 * Outputs, like surfaces, have a position and dimensions that allow them to be logically organized in a similar way to how a system settings panel does.\n
 * You can adjust the position of an output using the setPos() method, which will, in turn, trigger the moveGL() function when the position changes.
 *
 * <center><IMG height="350px" SRC="https://lh3.googleusercontent.com/VOWUX4iiqYMF_bIrBP3xMyaiydv_e_ZKznCIJlRLaEA0CtBLMuU4h41R3D4Xm-7krk8jFGZrQGb_SS7hlIFUY9E5dVbQqs0Q3NIBXvRFrGs_cukqOmbCv1ExN9fG3BDdj4Yz45xIkQ=w2400"></center>
 *
 * @note To enable LCursor to transition across different LOutputs, ensure that the outputs are closely arranged side by side.
 *
 * @section Uninitialization
 *
 * If an LOutput is no longer available, or you no longer wish to use it, you must call the LCompositor::removeOutput() method to
 * remove it from the compositor. This will only uninitialize its rendering thread, making it possible to re-initialize it later.\n
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
     * @brief Set the damaged region of the framebuffer.
     *
     * This method is used to specify which region of the framebuffer has been damaged within a paintGL() call.
     * The damage region is cleared after the subsequent paintGL() call.
     *
     * @note Calling this method is not mandatory, but it could considerably improve performance on certain graphic backends/hardware.
     *
     * @param damage The damaged region of the framebuffer.
     */
    void setBufferDamage(const LRegion &damage);

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
    void setScale(Int32 scale);

    /**
     * @brief Retrieve the current output scale factor.
     *
     * This function returns the current scale factor assigned to the output using setScale(). The default scale factor is 1.
     */
    Int32 scale() const;

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
     * This function retrieves the name of the output provided by the graphic backend, such as "HDMI-A-2."
     *
     * @note Output names are always unique, even if they belong to different GPUs.
     */
    const char *name() const;

    /**
     * @brief Get the output model name.
     *
     * This function retrieves the model name of the output provided by the graphic backend.
     */
    const char *model() const;

    /**
     * @brief Get the manufacturer name of the output.
     *
     * This function retrieves the manufacturer name of the output provided by the graphic backend.
     */
    const char *manufacturer() const;

    /**
     * @brief Get the description of the output.
     *
     * This function retrieves the description of the output provided by the graphic backend.
     */
    const char *description() const;

    /**
     * @brief Get access to the associated LPainter.
     *
     * This function provides access to the LPainter associated with this output.
     */
    LPainter *painter() const;

    /**
     * @brief Get the ID of the rendering thread.
     *
     * This function retrieves the ID of the output rendering thread.
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

    LPRIVATE_IMP(LOutput)
};

#endif // LOUTPUT_H
