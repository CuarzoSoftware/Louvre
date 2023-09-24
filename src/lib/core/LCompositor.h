#ifndef LCOMPOSITOR_H
#define LCOMPOSITOR_H

#include <LClient.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LDNDManager.h>
#include <LPopupRole.h>
#include <LSubsurfaceRole.h>
#include <LCursorRole.h>
#include <LDNDIconRole.h>

#include <thread>

/*!
 * @brief Louvre's primary class and resource factory.
 *
 * The LCompositor class initializes Louvre's Wayland event loop and backend systems.\n
 * After calling start(), the initialized() virtual method is invoked to indicate successful initialization.
 * Any initial setup should be done there or later to prevent issues.
 *
 * LCompositor also follows the factory design pattern, using virtual methods as constructors and destructors for various library classes.
 * Each of those classes has multiple virtual methods that handle events.
 * To customize a class's behavior for a specific event, subclass it and override the corresponding virtual method.
 * To make the library use your custom subclass, override its virtual constructor in LCompositor.
 *
 * For example, consider the LOutput class, created within LCompositor::createOutputRequest().
 * To use your custom LOutput subclass, override LCompositor::createOutputRequest() and return an instance of your subclass.
 * This way, the library will use your custom LOutput subclass for output-related events.
 */
class Louvre::LCompositor
{
public:

    /// Possible compositor states
    enum CompositorState
    {
        /// Uninitialized
        Uninitialized,

        /// Changing from uninitialized to initialized
        Initializing,

        /// Initialized
        Initialized,

        /// Changing from any state to uninitialized
        Uninitializing,

        /// Changing from initialized to paused
        Pausing,

        /// Idle state for example when changing session
        Paused,

        /// Changing from paused to initialized
        Resuming
    };

    /*!
     * @brief Constructor of the LCompositor class.
     */
    LCompositor();

    /*!
     * @brief Destructor of the LCompositor class.
     */
    virtual ~LCompositor();

    /// @cond OMIT
    LCompositor(const LCompositor&) = delete;
    LCompositor& operator= (const LCompositor&) = delete;
    /// @endcond

    /*!
     * @brief Get the static LCompositor instance.
     *
     * This method provides access to the single LCompositor instance that can exist per process. You can use it from any part of your code.
     *
     * @return A pointer to the LCompositor instance.
     */
    static LCompositor *compositor();

    /*!
     * @brief Override to add custom Wayland globals during compositor initialization or modify default globals.
     *
     * Override this method if you want to add custom Wayland globals when initializing the compositor or if you wish to modify the default globals or their versions.
     *
     * @return `true` on success, `false` on failure (prevents the compositor from starting).
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createGlobalsRequest
     */
    virtual bool createGlobalsRequest();

    /*!
     * @brief Loads a graphical backend (dynamic library).
     *
     * Use this method to load a custom graphical backend. By default, the library loads the DRM backend located at `/usr/etc/Louvre/backends/libLGraphicBackendDRM.so` if this method is not used.
     *
     * @param path Location of the backend's dynamic library.
     *
     * @return `true` if the backend is successfully loaded, `false` otherwise.
     */
    bool loadGraphicBackend(const char *path);

    /*!
     * @brief Checks if the graphical backend is initialized.
     *
     * Use this method to determine whether the graphical backend has been initialized after calling the start() function.
     *
     * @return `true` if the graphical backend is initialized, `false` otherwise.
     */
    bool isGraphicBackendInitialized() const;

    /*!
     * @brief Loads an input backend (dynamic library).
     *
     * Use this method to load a cusom input backend. By default, the library loads the Libinput backend located at `/usr/etc/Louvre/backends/libLInputBackendLibinput.so` if this method is not used.
     *
     * @param path Location of the backend's dynamic library.
     *
     * @return `true` if the backend is successfully loaded, `false` otherwise.
     */
    bool loadInputBackend(const char *path);

    /*!
     * @brief Checks if the input backend is initialized.
     *
     * Use this method to determine whether the input backend has been initialized after calling the start() function.
     *
     * @return `true` if the input backend is initialized, `false` otherwise.
     */
    bool isInputBackendInitialized() const;

    /*!
     * @brief Notifies a successful compositor initialization.
     *
     * Use this method to handle the successful initialization of the compositor after calling the start() method.
     * Here you should perform initial configuration tasks, such as setting up outputs, as demonstrated in the default implementation.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp initialized
     */
    virtual void initialized();

    /*!
     * @brief Notifies a successful cursor initialization.
     *
     * Use this method to handle the successful initialization of the cursor.
     * It is recommended to load cursor textures within this method.
     * The LXCursor class provides the LXCursor::loadXCursorB() method for loading pixmaps of X cursors available on the system.
     *
     * The default implementation includes a commented example demonstrating how to load X cursor pixmaps and assign them to the cursor.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp cursorInitialized
     */
    virtual void cursorInitialized();

    /*!
     * @brief Gets the current compositor state.
     *
     * Use this function to retrieve the current state of the compositor. You can check the LCompositor::CompositorState enum for possible state values.
     *
     * @return The current compositor state.
     */
    CompositorState state() const;

    /*!
     * @name Virtual Constructors
     * Virtual constructors are used to dynamically instantiate variants of a base or abstract class.\n
     * The compositor uses virtual constructors to create and keep track of resources like LSurface, LOutput, LKeyboard, etc,
     * and for letting you use your own subclasses and override their virtual methods.
     */

///@{

    /*!
     * @brief Virtual constructor for creating LOutput instances when needed by the graphical backend.
     *
     * This method is invoked by the graphical backend when it needs to create a new output.
     *
     * @return An instance of LOutput or a subclass of LOutput.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createOutputRequest
     */
    virtual LOutput *createOutputRequest();

    /*!
     * @brief Virtual constructor for creating LClient instances when a new client connects.
     *
     * This method is called when a new client establishes a connection with the compositor.
     *
     * @param params Internal Louvre parameters for creating the client.
     *
     * @return An instance of LClient or a subclass of LClient.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createClientRequest
     */
    virtual LClient *createClientRequest(LClient::Params *params);

    /*!
     * @brief Virtual constructor for creating LSurface instances when a client creates a new surface.
     *
     * This method is called when a client creates a new surface. The `LSurface` class and its subclasses provide virtual methods to notify changes in roles, size, mapping, and more.
     *
     * @param params Internal Louvre parameters for creating the surface.
     *
     * @return An instance of LSurface or a subclass of LSurface.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createSurfaceRequest
     */
    virtual LSurface *createSurfaceRequest(LSurface::Params *params);

    /*!
     * @brief Virtual constructor for creating the LSeat instance during compositor initialization.
     *
     * This method is called during the compositor initialization. The LSeat class provides virtual methods to access native events generated by the input backend, handle output hotplugging events, TTY switching events, and more.
     *
     * @param params Internal Louvre parameters for creating the seat.
     *
     * @return An instance of LSeat or a subclass of LSeat.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createSeatRequest
     */
    virtual LSeat *createSeatRequest(LSeat::Params *params);

    /*!
     * @brief Virtual constructor for creating the LPointer instance during LSeat initialization.
     *
     * This method is called during LSeat initialization. The LPointer class provides virtual methods to listen to pointer events generated by the input backend.
     *
     * @param params Internal Louvre parameters for creating the pointer.
     *
     * @return An instance of LPointer or a subclass of LPointer.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createPointerRequest
     */
    virtual LPointer *createPointerRequest(LPointer::Params *params);

    /*!
     * @brief Virtual constructor for creating the LKeyboard instance during LSeat initialization.
     *
     * This method is called during LSeat initialization. The LKeyboard class provides virtual methods to listen to keyboard events generated by the input backend.
     *
     * @param params Internal Louvre parameters for creating the keyboard.
     *
     * @return An instance of LKeyboard or a subclass of LKeyboard.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createKeyboardRequest
     */
    virtual LKeyboard *createKeyboardRequest(LKeyboard::Params *params);

    /*!
     * @brief Virtual constructor for creating the LDNDManager instance during LSeat initialization.
     *
     * This method is called during LSeat initialization.
     * The LDNDManager class provides virtual methods that notify the start and end of
     * Drag & Drop sessions between clients.
     *
     * @param params Internal Louvre parameters for creating the DND manager.
     *
     * @return An instance of LDNDManager or a subclass of LDNDManager.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createDNDManagerRequest
     */
    virtual LDNDManager *createDNDManagerRequest(LDNDManager::Params *params);

    /*!
     * @brief Virtual constructor for creating LToplevelRole instances when a client creates a Toplevel role for a surface.
     *
     * This method is called when a client creates a Toplevel role for a surface.
     * The LToplevelRole class provides virtual methods to notify changes in geometry, state (activated, maximized, fullscreen, etc.),
     * the start of interactive moving and resizing sessions, and more.
     *
     * @param params Internal Louvre parameters for creating the Toplevel role.
     *
     * @return An instance of LToplevelRole or a subclass of LToplevelRole.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createToplevelRoleRequest
     */
    virtual LToplevelRole *createToplevelRoleRequest(LToplevelRole::Params *params);

    /*!
     * @brief Virtual constructor for creating LPopupRole instances when a client creates a Popup role for a surface.
     *
     * This method is called when a client creates a Popup role for a surface.
     * The LPopupRole class provides virtual methods to notify changes in geometry, repositioning based on its LPositioner,
     * input grabbing requests, and more.
     *
     * @param params Internal Louvre parameters for creating the Popup role.
     *
     * @return An instance of LPopupRole or a subclass of LPopupRole.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createPopupRoleRequest
     */
    virtual LPopupRole *createPopupRoleRequest(LPopupRole::Params *params);

    /*!
     * @brief Virtual constructor for creating LSubsurfaceRole instances when a client creates a Subsurface role for a surface.
     *
     * This method is called when a client creates a Subsurface role for a surface.
     * The LSubsurfaceRole class provides virtual methods to notify changes in its local position relative to its parent,
     * rearrangement on the stack of neighboring surfaces, and more.
     *
     * @param params Internal Louvre parameters for creating the Subsurface role.
     *
     * @return An instance of LSubsurfaceRole or a subclass of LSubsurfaceRole.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createSubsurfaceRoleRequest
     */
    virtual LSubsurfaceRole *createSubsurfaceRoleRequest(LSubsurfaceRole::Params *params);

    /*!
     * @brief Virtual constructor for creating LCursorRole instances when a client wants to use a surface as a cursor.
     *
     * This method is called when a client wants to use a surface as a cursor by invoking the LPointer::setCursorRequest() method.
     * The LCursorRole class provides a virtual method that notifies changes in the hotspot of the cursor.
     *
     * @param params Internal Louvre parameters for creating the Cursor role.
     *
     * @return An instance of LCursorRole or a subclass of LCursorRole.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createCursorRoleRequest
     */
    virtual LCursorRole *createCursorRoleRequest(LCursorRole::Params *params);

    /*!
     * @brief Virtual constructor for creating LDNDIconRole instances when a client wants to use a surface as an icon for a Drag & Drop session.
     *
     * This method is called when a client wants to use a surface as an icon for a Drag & Drop session.
     * Similar to LCursorRole, the LDNDIconRole class provides a virtual method that notifies changes in the hotspot of the icon being dragged.
     *
     * @param params Optional parameters for creating the DND icon role.
     *
     * @return An instance of LDNDIconRole or a subclass of LDNDIconRole.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createDNDIconRoleRequest
     */
    virtual LDNDIconRole *createDNDIconRoleRequest(LDNDIconRole::Params *params);

///@}

    /*!
     * @name Virtual Destructors
     * Virtual destructors are used by the compositor to provide early notification of the destruction of a resource.\n
     *
     * @warning The compositor internally handles the freeing of memory after the destructor is called, 
     * so you must not call `delete` on the resource provided in the argument.
     * Only memory allocated by yourself must be freed separately since its not freed by the library.
     */

///@{

    /*!
     * @brief Virtual destructor for the LOutput class.
     *
     * This method is invoked by the graphical backend when an output becomes unavailable.
     *
     * @note If the output was initialized, you can be sure its LOutput::uninitializeGL() virtual method is invoked before this.
     *
     * @param output The LOutput instance to be destroyed.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp destroyOutputRequest
     */
    virtual void destroyOutputRequest(LOutput *output);

    /*!
     * @brief Virtual destructor for the LClient class.
     *
     * This method is called when a client disconnects, and it is invoked after all its resources have been released.
     *
     * @param client The LClient instance to be destroyed.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp destroyClientRequest
     */
    virtual void destroyClientRequest(LClient *client);

    /*!
     * @brief Virtual destructor for the LSurface class.
     *
     * This method is called when a client requests to destroy one of its surfaces.
     *
     * @param surface The LSurface instance to be destroyed.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp destroySurfaceRequest
     */
    virtual void destroySurfaceRequest(LSurface *surface);

    /*!
     * @brief Virtual destructor for the LToplevelRole class.
     *
     * This method is called when a client requests to destroy the Toplevel role of one of its surfaces.
     *
     * @param toplevel The LToplevelRole instance to be destroyed.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp destroyToplevelRoleRequest
     */
    virtual void destroyToplevelRoleRequest(LToplevelRole *toplevel);

    /*!
     * @brief Virtual destructor for the LPopupRole class.
     *
     * This method is called when a client requests to destroy the Popup role of one of its surfaces.
     *
     * @param popup The LPopupRole instance to be destroyed.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp destroyPopupRoleRequest
     */
    virtual void destroyPopupRoleRequest(LPopupRole *popup);

    /*!
     * @brief Virtual destructor for the LSubsurfaceRole class.
     *
     * This method is called when a client requests to destroy the Subsurface role of one of its surfaces.
     *
     * @param subsurface The LSubsurfaceRole instance to be destroyed.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp destroySubsurfaceRoleRequest
     */
    virtual void destroySubsurfaceRoleRequest(LSubsurfaceRole *subsurface);

    /*!
     * @brief Virtual destructor for the LCursorRole class.
     *
     * This method is invoked when a client requests to destroy the Cursor role of one of its surfaces.
     *
     * @param cursor The LCursorRole instance to be destroyed.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp destroyCursorRoleRequest
     */
    virtual void destroyCursorRoleRequest(LCursorRole *cursor);

    /*!
     * @brief Virtual destructor for the LDNDIconRole class.
     *
     * This method is invoked when a client requests to destroy the DNDIcon role of one of its surfaces.
     *
     * @param icon The LDNDIconRole instance to be destroyed.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp destroyDNDIconRoleRequest
     */
    virtual void destroyDNDIconRoleRequest(LDNDIconRole *icon);


///@}

    /*!
     * @brief Starts the Wayland event loop and backends.
     *
     * After notifying its successful initialization with initialized(),
     * the compositor becomes capable of receiving connections from Wayland clients and initializing
     * output rendering threads with the addOutput() method.
     *
     * @return `false` on failure and `true` otherwise.
     */
    bool start();

    /*!
     * @brief Process the compositor's main event loop.
     *
     * You can also use a pollable file descriptor to listen to events, which can be accessed with fd().
     *
     * @param msTimeout Milliseconds to wait before an event occurs. Setting it to 0 disables the timeout, and setting it to -1 makes it wait indefinitely until an event occurs.
     *
     * @return 1 on success and 0 on failure.
     */
    Int32 processLoop(Int32 msTimeout);

    /*!
     * @brief Get the pollable file descriptor of the main event loop.
     *
     * @return The pollable file descriptor.
     */
    Int32 fd() const;

    /*!
     * @brief Ends and uninitializes the compositor.
     */
    void finish();

    /*!
     * @brief Get the native wl_display used by the compositor.
     *
     * @return The native wl_display.
     */
    static wl_display *display();

    /*!
     * @brief Get the main wl_event_loop used by the compositor.
     *
     * @return The main wl_event_loop.
     */
    static wl_event_loop *eventLoop();

    /*!
     * @brief Add a pollable FD to the compositor's event loop.
     *
     * @param fd The file descriptor to be added.
     * @param userData User data to pass to the callback function.
     * @param callback The callback function to handle events on the file descriptor.
     * @param flags Flags to specify the type of event to listen for (e.g., WL_EVENT_READABLE).
     *
     * @return The wl_event_source associated with the added file descriptor.
     */
    static wl_event_source *addFdListener(int fd, void *userData, int(*callback)(int,unsigned int,void*), UInt32 flags = WL_EVENT_READABLE);

    /*!
     * @brief Remove a previously added pollable FD from the compositor's event loop.
     *
     * @param source The wl_event_source to remove.
     */
    static void removeFdListener(wl_event_source *source);

    /*!
     * @brief Get a pointer to the cursor implementation provided by the library.
     *
     * This method must be accessed within or after initialized() or cursorInitialized()` are invoked.
     * If the cursor has not yet been initialized, this method returns `nullptr`.
     *
     * @return A pointer to the LCursor instance or `nullptr` if not yet initialized.
     */
    LCursor *cursor() const;

    /*!
     * @brief Get a pointer to the compositor seat.
     *
     * The seat provides access to LPointer, LKeyboard, LTouch, and LOutput instances.
     * This method must be accessed after the compositor initialization.
     *
     * @return A pointer to the LSeat instance.
     */
    LSeat *seat() const;

    /*!
     * @brief Schedule a new rendering frame for each output in the compositor.
     *
     * This function schedules a new rendering frame by calling the `LOutput::repaint()` method for all initialized outputs.
     * The order in which the outputs are rendered is undefined since each one works in its own thread.
     */
    void repaintAllOutputs();

    /*!
     * @brief Initializes the specified output for rendering.
     *
     * This function initializes the specified output for rendering, allowing you to schedule rendering frames
     * using the LOutput::repaint() method. The list of initialized outputs can be accessed with outputs().
     * Adding an already initialized output is a no-op.
     *
     * @param output The output to initialize, obtained from LSeat::outputs().
     *
     * @return `true` on success, `false` on failure.
     */
    bool addOutput(LOutput *output);

    /*!
     * @brief Uninitializes the specified output.
     *
     * This function uninitializes and removes the specified output from the compositor, stopping its thread and rendering loop.
     * Removing an output that has not been added to the compositor is a no-op.
     *
     * @warning Calling this method from the same rendering thread of the output (during any of its `...GL()` events) has no effect. Doing so would lead to a deadlock, so the Library simply ignores the request.
     *
     * @param output The output to remove, previously added to the compositor and accessible from `outputs()`.
     */
    void removeOutput(LOutput *output);

    /*!
     * @brief Get a list of all surfaces created by clients.
     *
     * This function returns a list of all surfaces created by clients, respecting the stacking order of their roles/protocols.
     * To access surfaces created by a specific client, use the LClient::surfaces() method instead.
     *
     * @return A list of LSurface objects representing the surfaces.
     */
    const std::list<LSurface*> &surfaces() const;

    /*!
     * @brief Get a list of all initialized outputs.
     *
     * This function returns a list of all outputs that have been initialized using the addOutput() method.
     *
     * @note This list only contains initialized outputs. To get a list of all available outputs, use LSeat::outputs() instead.
     *
     * @return A list of LOutput objects representing the initialized outputs.
     */
    const std::list<LOutput*>& outputs() const;

    /*!
     * @brief Get a list of clients connected to the compositor.
     *
     * This function returns a list of clients that are currently connected to the compositor.
     *
     * @return A list of LClient objects representing the connected clients.
     */
    const std::list<LClient*>& clients() const;

    /*!
     * @brief Get the next positive integer number in sequence.
     *
     * This function returns a new positive integer number each time it is called, incrementally.
     *
     * @return The next positive integer in the sequence.
     */
    static UInt32 nextSerial();

    /*!
     * @brief Get the main EGL display created by the graphical backend.
     *
     * This function returns the main EGL display created by the graphical backend.
     *
     * @return The main EGL display.
     */
    static EGLDisplay eglDisplay();

    /*!
     * @brief Get the main EGL context created by the graphical backend.
     *
     * This function returns the main EGL context created by the graphical backend.
     *
     * @return The main EGL context.
     */
    static EGLContext eglContext();

    /*!
     * @brief Flush all pending client events.
     *
     * This function immediatly flushes all pending client events.
     */
    static void flushClients();

    /*!
      * @brief Gets the LClient of a native Wayland `wl_client` resource.
      *
      * @returns The LClient instance for a `wl_client` resource or `nullptr` if not found.
      */
    LClient *getClientFromNativeResource(wl_client *client);

    /*!
     * @brief Identifier of the main thread.
     *
     * This ID corresponds to the primary thread responsible for managing the Wayland and input backend event loops,
     * while individual output operations are performed on separate threads.
     *
     * @return The identifier of the main thread.
     */
    std::thread::id mainThreadId() const;

    LPRIVATE_IMP(LCompositor)
};

#endif // LCOMPOSITOR_H
