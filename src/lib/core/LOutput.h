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

/*!
 * @brief Representation of a screen.
 *
 * The LOutput class represents a screen where part of the compositor can be displayed. It is typically associated with a computer monitor but
 * could also be a window of an X11 or Wayland desktop depending on the selected graphical backend.\n
 *
 * <center><IMG height="250px" SRC="https://lh3.googleusercontent.com/4lV1LTHBmO-eFywBrL4UhYIRcQbV5bjGB_17FdWFCzjGvnklxwBnXz5hQKOrkRCOegsn6PjnYZNCWk1SjFjwh9t8olEzr3Uwzd3saEt8EKRbbqX0n1f5R7q6r6V9u1t0PUk7BB0teA"></center>
 *
 * It has virtual methods used to initialize graphical contexts, rendering frames, adjusting the viewport, and more.\n
 *
 * @section Access
 *
 * The graphical backend is responsible for creating each LOutput making a request to the compositor through LCompositor::createOutputRequest().\n
 * You can reimplement that virtual constructor to use your own LOutput subclasses.\n
 * The LSeat class grants access to all outputs created by the graphical backend through the LSeat::outputs() list and it also notifies you of
 * hotplugging events (e.g. when connecting or disconnecting a monitor through an HDMI port).\n
 *
 * @section Initialization
 *
 * By default, outputs are inactive and therefore it is not possible to render on them. To activate an output, it must be added to the compositor
 * using the LCompositor::addOutput() method, this will initialize its rendering thread, graphical context, and invoke the initializeGL() method. Then, every time the
 * repaint() method is called, the next rendering frame will be scheduled and the paintGL() method will be invoked within which you can define your own rendering logic.\n
 * By default, the library initializes all available outputs once the compositor is started in the LCompositor::initialized() virtual method.
 *
 * @note The library internally handles the invocation of certain virtual methods: initializeGL(), moveGL(), resizeGL(), paintGL(), and uninitializeGL(). These methods are not intended to be called directly by you. To schedule a new frame, use the repaint() method, which will eventually trigger paintGL() asynchronously. It's important to note that making multiple calls to repaint() within the same frame does not result in multiple invocations of paintGL(). Instead, it ensures that paintGL() is invoked once, right after the subsequent paintGL(), repaint() should be called again in order to schedule another frame.
 *
 * @section Modes
 *
 * Each LOutput can have multiple modes. An LOutputMode contains information about the resolution and refresh rate that the output can operate at.\n 
 * You can access the modes of an output with the modes() method and set the desired mode with setMode().\n
 * Generally, outputs use the mode with the highest refresh rate and resolution by default.\n
 * If you change to a mode with a different resolution and the output is initialized, the resizeGL() method will be invoked.
 *
 * @section Context
 *
 * Each output has its own OpenGL context and its own instance of LPainter, which can be accessed with the painter() method.\n
 * You can use the functions provided by LPainter to render colored rects or textures.\n
 *
 * @note You can also use the native OpenGL functions and your own shaders/programs if you wish.
 *
 * @section Layout
 *
 * Outputs, like surfaces, have a position and dimensions that allow them to be logically organized in a similar way to how a system settings panel does.\n
 * You can adjust the position of an output using the setPos() method, which will, in turn, trigger the moveGL() function if the position changed.
 *
 * <center><IMG height="350px" SRC="https://lh3.googleusercontent.com/VOWUX4iiqYMF_bIrBP3xMyaiydv_e_ZKznCIJlRLaEA0CtBLMuU4h41R3D4Xm-7krk8jFGZrQGb_SS7hlIFUY9E5dVbQqs0Q3NIBXvRFrGs_cukqOmbCv1ExN9fG3BDdj4Yz45xIkQ=w2400"></center>
 *
 * @note In order for the LCursor class to smoothly transition across different LOutputs, the outputs must be arranged side by side.
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
     * @brief Get the current state of the LOutput.
     *
     * This method returns the current state of the output.
     *
     * @return The state of the output as a member of the State enumeration.
     */
    State state() const;

    /*!
     * @brief Constructor of the LOutput class.
     */
    LOutput();

    /*!
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
     * @brief Returns a pointer to the associated framebuffer.
     *
     * @return A pointer to the LFramebuffer instance associated with the output.
     */
    LFramebuffer *framebuffer() const;

    /**
     * @brief Returns the index of the current buffer.
     *
     * Compositors commonly employ double or triple buffering to ensure smooth graphics rendering. This involves rendering to one buffer while displaying another, reducing visual artifacts like glitches and tearing.
     *
     * @image html https://lh3.googleusercontent.com/2ousoWwxnVGvFX5bT6ual2G8UUbhUOJ21mK1UQmthPNM-7XfracRlL5GCYBQTzt4Os28eKO_FzC6BS-rasiNngvTMI9lEdET0ItKrI2wK_9IwSDaF-hNGkTMI6gVlL0m4ENDJYbckw
     *
     * @return The current buffer index. Typically alternates between 0 and 1, although some backends may employ less or more than 2 framebuffers.
     */
    Int32 currentBuffer() const;

    /**
     * @brief Returns the count of available buffers.
     *
     * This method provides the number of available buffers in the output, which are used for double buffering and rendering optimizations.
     *
     * @return The count of available buffers.
     */
    UInt32 buffersCount() const;

    /**
     * @brief Accesses the texture of a specific buffer.
     *
     * This method allows access to the texture associated with a particular buffer index.
     *
     * @param bufferIndex The index of the buffer for which the texture is to be accessed.
     * @return A pointer to the texture associated with the specified buffer index, or nullptr if texture access is not supported.
     *
     * @warning Some hardware may not support accessing textures, so you should always check if nullptr is returned.
     */
    LTexture *bufferTexture(UInt32 bufferIndex);

    /**
     * @brief Checks if the graphical backend supports buffer damage tracking.
     *
     * Some graphical backends / hardware can benefit from knowing which regions of the framebuffer have changed after a paintGL() call. This method indicates whether buffer damage support is available.
     *
     * @return true if the graphical backend supports buffer damage tracking, false otherwise.
     */
    bool hasBufferDamageSupport() const;

    /**
     * @brief Sets the damaged region of the framebuffer.
     *
     * This method is used to specify which region of the framebuffer has been damaged after a paintGL() call. The damage region is cleared after the subsequent paintGL() call.
     *
     * @note Calling this method is not mandatory, but it could considerably improve performance on certain hardware.
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

    /*!
     * @brief Preferred mode.
     *
     * Preferred mode for the output. It is generally the mode with the highest refresh rate and resolution.
     */
    const LOutputMode *preferredMode() const;

    /*!
     * @brief Current mode.
     *
     * The graphical backend assigns the preferred mode by default.
     */
    const LOutputMode *currentMode() const;

    /*!
     * @brief Sets an output mode.
     *
     * Assigns a mode to the output. It must be one of the modes available in the modes().\n
     * If the output is initialized and the new mode has a different resolution than the current one, the resizeGL() method is invoked.
     */
    void setMode(const LOutputMode *mode);

    /*!
     * @brief Sets the output scale.
     *
     * Some monitors have a high pixel density, which means that applications need to render their content at a higher resolution in order to be viewed in high definition. 
     * Typically, a dpi() value greater than 120 is considered high definition, and in such cases, it is recommended to use a scale factor of 2. 
     * Clients automatically adjusts the scale of their surfaces to match the output on which they are visible.\n
     * For more information, you can refer to the default implementation of the LCompositor::initialized() method.
     */
    void setScale(Int32 scale);

    /*!
     * @brief Current scale.
     *
     * @returns the current scale of the output assigned with setScale(). The default scale is 1.
     */
    Int32 scale() const;

    /*!
     * @brief Schedules the next rendering frame.
     *
     * This method requests the compositor to schedule the next rendering frame for this output.\n
     * The compositor will then invoke the paintGL() method during the next frame.\n
     * Calling this method multiple times during the same frame does not result in multiple invocations of the paintGL() method, it only ensures that it will be invoked once in the next frame.
     */
    void repaint();

    /*!
     * @brief Dots per inch.
     *
     * Dots per inch of the output, taking into account its physical dimensions and the resolution given by its current mode.
     */
    Int32 dpi();

    /*!
     * @brief Physical dimensions of the output.
     *
     * Physical dimensions of the output in millimeters.
     */
    const LSize &physicalSize() const;

    /*!
     * @brief Output size in compositor coordinates.
     *
     * Size of the output given its current mode in surface coordinates. Equivalent to the size given by rect().
     */
    const LSize &size() const;

    /*!
     * @brief Output size in buffer coordinates.
     *
     * Size of the output given its current mode in buffer coordinates.
     */
    const LSize &sizeB() const;

    /*!
     * @brief Output rect.
     *
     * Position and size of the output in surface coordinates.
     */
    const LRect &rect() const;

    /*!
     * @brief Output position in surface coordinates.
     *
     * Position of the output in surface coordinates. Equivalent to the position given by rect().
     */
    const LPoint &pos() const;

    /*!
     * @brief Assigns the position of the output.
     *
     * Sets the position of the output in surface coordinates, with the upper-left corner as the origin.
     */
    void setPos(const LPoint &pos);

    /*!
     * @brief EGLDisplay handle.
     *
     * Handle to the EGLDisplay of the output created by the graphical backend.
     */
    EGLDisplay eglDisplay();

    /*!
     * @brief Output name.
     *
     * Name of the output given by the graphical backend (e.g HDMI-A-2).
     */
    const char *name() const;

    /*!
     * @brief Output model
     *
     * Model name of the output given by the graphical backend.
     */
    const char *model() const;

    /*!
     * @brief Manufacturer of the output.
     *
     * Manufacturer name of the output given by the graphical backend.
     */
    const char *manufacturer() const;

    /*!
     * @brief Description of the output.
     *
     * Description of the output given by the graphical backend.
     */
    const char *description() const;

    /*!
     * @brief Renderer
     *
     * Access to render functions.
     */
    LPainter *painter() const;

    /*!
     * @brief Rendering thread ID
     */
    const std::thread::id &threadId() const;

    /*!
     * @name Métodos Virtuales
     */
///@{

    /*!
     * @brief Initialization of the OpenGL context.
     *
     * The initializeGL() method is invoked by the library after the output is properly initialized.\n
     * You can reimplement this method to initialize your shaders, programs, textures, etc.\n
     *
     * The default implementation assigns the white color to clear the screen.
     *
     * @note You should avoid performing any painting operations here, as they won't be visible on the screen. Instead, perform painting tasks in the paintGL() method.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp initializeGL
     */
    virtual void initializeGL();

    /*!
     * @brief Renders a frame.
     *
     * The paintGL() method is invoked by the library after calling the repaint() method.\n
     * Reimplement this method to define your own rendering logic.
     *
     * The default implementation clears the screen, draws the surfaces following the order of the surface list
     * of the compositor, draws the cursor if the graphical backend does not support cursor composition via hardware and
     * finally draws the icon of a drag & drop session if there was one.
     *
     * @warning The default implementation provides a basic rendering method that is quite inefficient since it redraws the entire screen every frame. Check the code of the **louvre-weston-clone** example compositor to see how to render efficiently with LPainter or consider using the LScene rendering system.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp paintGL
     */
    virtual void paintGL();

    /*!
     * @brief Framebuffer dimensions change.
     *
     * The resizeGL() method is invoked by the library when the output scale or mode changes.\n
     * You can reimplement this method to readjust your graphical interfaces.\n
     *
     * @note You should avoid performing any painting operations here, as they won't be visible on the screen. Instead, perform painting tasks in the paintGL() method.
     *
     * The default implementation schedules a new rendering frame.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp resizeGL
     */
    virtual void resizeGL();

    /**
     * @brief Invoked when the output position changes.
     *
     * This method is called when the output's position changes, such as when the setPos() method is called.
     * You can reimplement this method to readjust your graphical interfaces or perform necessary updates in response to position changes.
     *
     * @note You should avoid performing any painting operations here, as they won't be visible on the screen. Instead, perform painting tasks in the paintGL() method.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp moveGL
     */
    virtual void moveGL();

    /*!
     * @brief OpenGL context deinitialization.
     *
     * The uninitializeGL() method is invoked by the library after the output is correctly removed from the compositor with LCompositor::removeOutput().\n
     * You can reimplement this method to deinitialize your shaders, programs, textures, etc.\n
     *
     * @note You should avoid performing any painting operations here, as they won't be visible on the screen. Instead, perform painting tasks in the paintGL() method.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp uninitializeGL
     */
    virtual void uninitializeGL();
///@}

    LPRIVATE_IMP(LOutput)
};

#endif // LOUTPUT_H
