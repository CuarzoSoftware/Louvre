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
 * @brief Representation of a graphical output (where rendering is performed).
 *
 * The LOutput class represents an output where part of the compositor can be displayed. It is typically associated with a computer screen but
 * could also be a window of an X11 or Wayland desktop depending on the selected graphic backend.\n
 *
 * <center><IMG height="250px" SRC="https://lh3.googleusercontent.com/4lV1LTHBmO-eFywBrL4UhYIRcQbV5bjGB_17FdWFCzjGvnklxwBnXz5hQKOrkRCOegsn6PjnYZNCWk1SjFjwh9t8olEzr3Uwzd3saEt8EKRbbqX0n1f5R7q6r6V9u1t0PUk7BB0teA=w2400"></center>
 *
 * It has virtual methods used to initialize graphical contexts, rendering frames, and adjusting the dimensions of the framebuffer.\n
 *
 * @section Access
 *
 * The graphic backend is responsible for creating each LOutput making a request to the compositor through LCompositor::createOutputRequest().\n
 * You can reimplement that virtual constructor to use your own LOutput subclass.\n
 * The LOutputManager class grants access to all outputs created by the graphic backend through the LOutputManager::outputs() list and notifies 
 * hotplugging events (e.g. when connecting or disconnecting a monitor through an HDMI port).\n
 *
 * @section Initialization
 *
 * By default, outputs are inactive and therefore it is not possible to render on them. To activate an output, it must be added to the compositor
 * using the LCompositor::addOutput() method, this will initialize its rendering thread, graphical context, and invoke the initializeGL() method. Then, every time the
 * repaint() method is called, the next rendering frame will be scheduled and the paintGL() method will be invoked within which you can define your own rendering logic.\n
 * By default, the library initializes all available outputs once the compositor is started in the LCompositor::initialized() virtual method.
 *
 * @warning The library takes care of calling the virtual methods of LOutput. To force the rendering of the next frame, you should use the repaint() method. Calling the repaint() method multiple times during the same frame does not mean that the paintGL() method will be invoked multiple times, it only ensures that it will be invoked in the next frame.
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
 * You can also use OpenGL functions and your own shaders/programs if you wish.
 *
 * @section Layout
 *
 * Outputs, like surfaces, have a position and dimensions that allow them to be logically organized in a similar way to how a system settings panel does.\n
 * You can set the position of an output using the setPosC() method.
 *
 * <center><IMG height="350px" SRC="https://lh3.googleusercontent.com/VOWUX4iiqYMF_bIrBP3xMyaiydv_e_ZKznCIJlRLaEA0CtBLMuU4h41R3D4Xm-7krk8jFGZrQGb_SS7hlIFUY9E5dVbQqs0Q3NIBXvRFrGs_cukqOmbCv1ExN9fG3BDdj4Yz45xIkQ=w2400"></center>
 *
 * @section Uninitialization
 *
 * If an LOutput is no longer available, or you no longer wish to use it, you must call the LCompositor::removeOutput() method to
 * remove it from the compositor. This will only uninitialize its rendering thread, making it possible to re-initialize it later.\n
 * An LOutput is no longer available when its virtual destructor is invoked (LCompositor::destroyOutputRequest()).
 *
 */

class Louvre::LOutput : public LObject
{
public:
    enum State
    {
        PendingInitialize = 0,
        PendingUninitialize = 1,
        Initialized = 2,
        Uninitialized = 3,
        ChangingMode = 4,
        Suspended = 5
    };

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

    LOutput(const LOutput&) = delete;
    LOutput& operator= (const LOutput&) = delete;

    /*!
     * @brief Modes list.
     *
     * List of avaliable modes of the output.
     */
    const list<LOutputMode *> *modes() const;

    /*!
     * @brief Preferred mode.
     *
     * Preferred mode for the output. It is generally the mode with the highest refresh rate and resolution.
     */
    const LOutputMode *preferredMode() const;

    /*!
     * @brief Current mode.
     *
     * The graphic backend assigns the preferred mode by default.
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
     * @brief Current buffer index.
     *
     * Generally, compositors use double buffering to render graphics. This involves rendering to one buffer while displaying the other, which helps to eliminate visual artifacts such as glitches and tearing.
     * 
     * <center><img src="https://lh3.googleusercontent.com/2ousoWwxnVGvFX5bT6ual2G8UUbhUOJ21mK1UQmthPNM-7XfracRlL5GCYBQTzt4Os28eKO_FzC6BS-rasiNngvTMI9lEdET0ItKrI2wK_9IwSDaF-hNGkTMI6gVlL0m4ENDJYbckw=w2400"></center>
     * @return It typically alternates between returning 0 or 1, but some backends may use more than 2 framebuffers.
     */
    Int32 currentBuffer() const;

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
     * @brief Output size in buffer coordinates.
     *
     * Size of the output given its current mode in buffer coordinates.
     */
    const LSize &sizeB() const;

    /*!
     * @brief Output size in compositor coordinates.
     *
     * Size of the output given its current mode in compositor coordinates. Equivalent to the size given by rectC().
     */
    const LSize &sizeC() const;

    /*!
     * @brief Output rect.
     *
     * Position and size of the output in compositor coordinates.
     */
    const LRect &rectC() const;

    /*!
     * @brief Output position in compositor coordinates.
     *
     * Position of the output in compositor coordinates. Equivalent to the position given by rectC().
     */
    const LPoint &posC() const;

    /*!
     * @brief Assigns the position of the output.
     *
     * Sets the position of the output in compositor coordinates, with the upper-left corner as the origin.
     */
    void setPosC(const LPoint &posC);

    /*!
     * @brief EGLDisplay handle.
     *
     * Handle to the EGLDisplay of the output created by the graphic backend.
     */
    EGLDisplay eglDisplay();

    State state() const;

    /*!
     * @brief Output name.
     *
     * Name of the output given by the graphic backend (e.g HDMI-2).
     */
    const char *name() const;

    /*!
     * @brief Output model
     *
     * Model name of the output given by the graphics backend.
     */
    const char *model() const;

    /*!
     * @brief Manufacturer of the output.
     *
     * Manufacturer name of the output given by the graphic backend.
     */
    const char *manufacturer() const;

    /*!
     * @brief Description of the output.
     *
     * Description of the output given by the graphic backend.
     */
    const char *description() const;

    /*!
     * @brief Renderer
     *
     * Access to render functions.
     */
    LPainter *painter() const;

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
     * of the compositor, draws the cursor if the graphic backend does not support cursor composition via hardware and
     * finally draws the icon of a drag & drop session if there was one.
     *
     * @warning The default implementation provides a basic rendering method that is quite inefficient since it redraws all the content every frame. It is recommended to analyze the code of **the louvre-weston-clone** example compositor to see how to render efficiently.
     * #### Default Implementation
     * @snippet LOutputDefault.cpp paintGL
     */
    virtual void paintGL();

    /*!
     * @brief Framebuffer dimensions change.
     *
     * The resizeGL() method is invoked by the library when the output mode changes.\n
     * You can reimplement this method to readjust your graphical interfaces.\n
     *
     * The default implementation schedules a new rendering frame.
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp resizeGL
     */
    virtual void resizeGL();

    /*!
     * @brief OpenGL context deinitialization.
     *
     * The uninitializeGL() method is invoked by the library after the output is correctly removed from the compositor with LCompositor::removeOutput().\n
     * You can reimplement this method to deinitialize your shaders, programs, textures, etc.\n
     *
     * #### Default Implementation
     * @snippet LOutputDefault.cpp uninitializeGL
     */
    virtual void uninitializeGL();
///@}

    LPRIVATE_IMP(LOutput)
};

#endif // LOUTPUT_H
