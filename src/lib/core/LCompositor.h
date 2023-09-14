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

using namespace std;

/*!
 * @brief Main class and Louvre's resources factory
 *
 * The LCompositor class is responsible for initializing the Wayland event loop and the input and graphic backend with the start() method.\n
 * The initialized() virtual method is called after start() to notify the correct initialization of the compositor.\n
 * Any initial configuration should be done within said method or later to avoid segmentation faults.
 *
 * LCompositor also follows the ***factory*** design pattern. It has virtual methods used
 * as virtual constructors and destructors of the many classes offered by the library.\n
 * Each class in the library contains multiple virtual methods that are invoked when events occur.
 * To change the default behavior of a class for a specific event, you must subclass it and override its corresponding
 * virtual method. In addition, in order for the library to use your customized subclass, you need to override its virtual
 * constructor in LCompositor.\n
 *
 * For example, let's consider the LOutput class, which is created within the LCompositor::createOutputRequest() virtual method.
 * If you want to use your own subclass of LOutput, you need to override LCompositor::createOutputRequest() and return an instance
 * of your customized LOutput subclass instead of a new LOutput instance. By doing so, the library will use your customized LOutput
 * subclass to handle output related events.
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
     * Constructor of the LCompositor class.
     */
    LCompositor();

    /*!
     * Destructor of the LCompositor class.
     */
    virtual ~LCompositor();

    LCompositor(const LCompositor&) = delete;
    LCompositor& operator= (const LCompositor&) = delete;

    /*!
     * Static LCompositor instance.
     */
    static LCompositor *compositor();

    /*!
     * Override this method if you want to add a custom global when initializing the compositor or if you
     * want to modify the default globals or their versions.
     *
     * @returns Must return **true** on success and **false** on failure. Returning **false** prevents the compositor to start.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createGlobalsRequest
     */
    virtual bool createGlobalsRequest();

    /*!
     * Loads a graphic backend (dynamic library) used to display the compositor. If this method is not used, the library will load
     * the DRM backend by default located at ```/usr/etc/Louvre/backends/libLGraphicBackendDRM.so```.\n
     *
     * @param path Location of the backend's dynamic library.
     *
     * @returns True if the backend is successfully loaded.
     */
    bool loadGraphicBackend(const char *path);

    /*!
     * Indicates whether the graphics backend is initialized after the start() function is called.
     */
    bool isGraphicBackendInitialized() const;

    /*!
     * Loads an input backend (dynamic library) to access events of input devices. If this method is not used, the library will load
     * the Libinput backend by default located at ```/usr/etc/Louvre/backends/libLInputBackendLibinput.so```.
     *
     * @param path Location of the backend's dynamic library.
     *
     * @returns True if the backend is loaded successfully.
     */
    bool loadInputBackend(const char *path);

    /*!
     * Indicates whether the graphics backend is initialized after the start() function is called.
     */
    bool isInputBackendInitialized() const;

    /*!
     * Notifies the successful initialization of the Wayland event loop and backends after calling the start() method.\n
     * This method should be used to perform the initial configuration of outputs as shown in the default implementation.
     * The available outputs can be accessed from the LSeat class instance, accessible with seat().
     * If you find an available output you can initialize it with the LCompositor::addOutput() method, and its own
     * OpenGL context and rendering thread will start (check the LOutput virtual methods).
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp initialized
     */
    virtual void initialized();

    /*!
     * Notifies the successful initialization of the cursor.\n
     * Within this method it is recommended to load cursor textures you may want to use. The LCursor class offers
     * the LXCursor::loadXCursorB() method for loading pixmaps of X cursors available on the system.\n
     * The default implementation shows a commented example on how to load X cursors pixmaps and assign them to the cursor.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp cursorInitialized
     */
    virtual void cursorInitialized();

    /*!
     * Returns the current compositor state. Check the LCompositor::CompositorState enum.
     */
    CompositorState state() const;

    /*!
     * @name Virtual Constructors
     * Virtual constructors are used to dynamically instantiate variants of a base or abstract class.\n
     * The compositor uses virtual constructors to create and keep track of resources like LSurfaces, LOutputs, LKeyboard, etc,
     * and for letting the developer use its own subclasses and override their virtual methods.
     */

///@{

    /*!
      * Virtual constructor of the LOutput class.
      * Called by the graphic backend for each available output.\n
      * The LOutput class has virtual methods to initialize graphic contexts and do rendering.
      * See the LOutput::initializeGL() and LOutput::paintGL() methods for more information.\n
      * Must return an instance of LOutput or a subclass of LOutput.
      *
      * #### Default implementation
      * @snippet LCompositorDefault.cpp createOutputRequest
      */
    virtual LOutput *createOutputRequest();

    /*!
     * Virtual constructor of the LClient class.
     * Called when a new client establishes a connection with the compositor.\n
     * The LClient class does not have virtual methods, therefore its reimplementation is not necessary.
     * However you can use this method to know when a client connects to the compositor.\n
     * Must return an instance of LClient or subclass of LClient.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createClientRequest
     */
    virtual LClient *createClientRequest(LClient::Params *params);

    /*!
     * Virtual constructor of the LSurface class. Called when a client creates a new surface.\n
     * The LSurface class has many virtual methods to notify changes of its role, size, mapping, etc.\n
     * Must return an instance of LSurface or a subclass of LSurface.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createSurfaceRequest
     */
    virtual LSurface *createSurfaceRequest(LSurface::Params *params);

    /*!
     * Virtual constructor of the LSeat class. Called during the compositor initialization.\n
     * The LSeat class has virtual method to access all events generated by the input and graphics backend.\n
     * Must return an instance of LSeat or a subclass of LSeat.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createSeatRequest
     */
    virtual LSeat *createSeatRequest(LSeat::Params *params);

    /*!
     * Virtual constructor of the LPointer class. Called during LSeat initialization.\n
     * The LPointer class has virtual methods to access pointer events generated by the input backend.\n
     * Must return an instance of LPointer or a subclass of LPointer.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createPointerRequest
     */
    virtual LPointer *createPointerRequest(LPointer::Params *params);

    /*!
     * Virtual constructor of the LKeyboard class. Called during LSeat initialization.\n
     * The LKeyboard class has virtual methods to access keyboard events generated by the input backend.\n
     * Must return an instance of LKeyboard or a subclass of LKeyboard.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createKeyboardRequest
     */
    virtual LKeyboard *createKeyboardRequest(LKeyboard::Params *params);

    /*!
     * Virtual constructor of the LDNDManager class. Called during LSeat initialization.\n
     * The LDNDManager class has virtual methods that notify the start and end of Drag & Drop sessions between clients.\n
     * Must return an instance of LDNDManager or a subclass of LDNDManager.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createDNDManagerRequest
     */
    virtual LDNDManager *createDNDManagerRequest(LDNDManager::Params *params);

    /*!
     * Virtual constructor of the LToplevelRole class.
     * Called when a client creates a Toplevel role for a surface.\n
     * The LToplevelRole class has virtual methods that notify,
     * its geometry change, state change (maximized, minimized, etc), the start
     * of interactive moving and resizing sessions, and so on.\n
     * Must return an instance of LToplevelRole or a subclass of LToplevelRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createToplevelRoleRequest
     */
    virtual LToplevelRole *createToplevelRoleRequest(LToplevelRole::Params *params);

    /*!
     * Virtual constructor of the LPopupRole class.
     * Called when a client creates a Popup role for a surface.\n
     * The LPopupRole class has virtual methods that notify,
     * its geometry change, its repositioning based on its LPositioner, input grabbing requests, and so on.\n
     * Must return an instance of LPopupRole or a subclass of LPopupRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createPopupRoleRequest
     */
    virtual LPopupRole *createPopupRoleRequest(LPopupRole::Params *params);

    /*!
     * Virtual constructor of the LSubsurfaceRole class.
     * Called when a client creates a Subsurface role for a surface.\n
     * The LSubsurfaceRole class has virtual methods that notify its local position change
     * relative to its parent, its rearrangement on the stack of neighboring surfaces, and more.\n
     * Must return an instance of LSubsurfaceRole or a subclass of LSubsurfaceRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createSubsurfaceRoleRequest
     */
    virtual LSubsurfaceRole *createSubsurfaceRoleRequest(LSubsurfaceRole::Params *params);

    /*!
     * Virtual constructor of the LCursorRole class.
     * Called when a client wants to use a surface as a cursor calling the LPointer::setCursorRequest() method.\n
     * The LCursorRole class has a virtual method that notifies the hotspot change
     * of the cursor being rendered.\n
     * Must return an instance of LCursorRole or a subclass of LCursorRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createCursorRoleRequest
     */
    virtual LCursorRole *createCursorRoleRequest(LCursorRole::Params *params);

    /*!
     * Virtual constructor of the LDNDIconRole class.
     * Called when a client wants to use a surface as an icon for a Drag & Drop session.\n
     * Similar to LCursorRole, the LDNDIconRole class has a virtual method that notifies the hotspot change
     * of the icon being dragged.\n
     * Must return an instance of LDNDIconRole or a subclass of LDNDIconRole.
     *
     * #### Default implementation
     * @snippet LCompositorDefault.cpp createDNDIconRoleRequest
     */
    virtual LDNDIconRole *createDNDIconRoleRequest(LDNDIconRole::Params *params);

///@}

    /*!
     * @name Virtual Destructors
     * Virtual destructors are used by the compositor to provide early notification of the destruction of a resource.\n
     *
     * @warning The compositor internally handles the freeing of memory after the destructor is called, 
     * so the developer should not call the "delete" method on the resource provided in the argument. 
     * Only memory managed by the developer must be freed separately since its not freed by the library.
     */

///@{

    /*!
     * Virtual destructor of the LOutput class.
     * Invoked by the graphic backend when an output becomes unavailable.\n
     * The virtual method LSeat::outputUnplugged() and the removeOutput()
     * method should be used to unlink the output from the compositor, stop its thread and render loop, 
     * and this virtual method to free any memory allocated in the LOutput reimplementation.
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyOutputRequest
     */
    virtual void destroyOutputRequest(LOutput *output);

    /*!
     * Virtual destructor of the LClient class.
     * Called when a client disconnects and after all its resources
     * have been released.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyClientRequest
     */
    virtual void destroyClientRequest(LClient *client);

    /*!
     * Virtual destructor of the LSurface class.
     * Called when a client requests to destroy one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroySurfaceRequest
     */
    virtual void destroySurfaceRequest(LSurface *surface);

    /*!
     * Virtual destructor of the LToplevelRole class.
     * Called when a client requests to destroy the Toplevel role of one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyToplevelRoleRequest
     */
    virtual void destroyToplevelRoleRequest(LToplevelRole *toplevel);

    /*!
     * Virtual destructor of the LPopupRole class.
     * Called when a client requests to destroy the Popup role of one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyPopupRoleRequest
     */
    virtual void destroyPopupRoleRequest(LPopupRole *popup);

    /*!
     * Virtual destructor of the LCursorRole class.
     * Invoked when a client requests to destroy the Cursor role of one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyCursorRoleRequest
     */
    virtual void destroyCursorRoleRequest(LCursorRole *cursor);

    /*!
     * Virtual destructor of the LDNDIconRole class.
     * Invoked when a client requests to destroy the DNDIcon role of one of its surfaces.\n
     * #### Default implementation
     * @snippet LCompositorDefault.cpp destroyDNDIconRoleRequest
     */
    virtual void destroyDNDIconRoleRequest(LDNDIconRole *icon);

///@}

    /*!
     * Starts the Wayland event loop and backends. After notifying its successful initialization with initialized(), the
     * compositor is able to receive connections from Wayland clients and initialize output rendering threads with the addOutput() method.\n
     *
     * @returns **false** on failure and **true** otherwise.
     */
    bool start();

    /*!
     * Process the compositor's main events loop.\n
     *
     * You can also use a pollable file descriptor (fd) to listen to events, which can be accessed with fd().
     *
     * @returns 1 on success and 0 on failure
     * @param msTimeout Milliseconds to wait before an event occurs. Setting it to 0 disables timeout, and setting it to -1 makes it wait indefinitely until an event occurs.
     */
    Int32 processLoop(Int32 msTimeout);

    /*!
     * Pollable file descriptor of the main event loop.
     */
    Int32 fd() const;

    /*!
     * Ends and uninitializes the compositor.
     */
    void finish();

    /*!
     * Native wl_display used by the compositor.
     */
    static wl_display *display();

    /*!
     * Main wl_event_loop used by the compositor.
     */
    static wl_event_loop *eventLoop();

    /*!
     * Add a pollable FD to the compositor's event loop.
     */
    static wl_event_source *addFdListener(int fd, void *userData, int(*callback)(int,unsigned int,void*), UInt32 flags = WL_EVENT_READABLE);

    /*!
     * Remove a previously added pollable FD from the compositor's event loop.
     */
    static void removeFdListener(wl_event_source *source);

    /*!
     * Returns a pointer to the cursor implementation provided by the library.\n
     * Must be accessed within or after initialized() or cursorInitialized() are invoked.\n
     * If the cursor has not yet been initialized, this method returns nullptr.
     */
    LCursor *cursor() const;

    /*!
     * Returns a pointer to the compositor seat from which the LPointer, LKeyboard, LTouch and LOutput instances can be accessed.\n
     * Must be accessed after compositor initialization.
     */
    LSeat *seat() const;

    /*!
     * This function calls the LOutput::repaint() method of all outputs added to the compositor.\n
     * The order in which the outputs are rendered is undefined since each one works in its own thread.\n
     */
    void repaintAllOutputs();

    /*!
     * Creates a new thread and render loop for the output and initializes it. Once initialized, the graphical backend invokes
     * the virtual method LOutput::initializeGL() to perform an initial setup and it is possible to call the
     * LOutput::repaint() method to schedule rendering frames.\n
     * The list of outputs added to the compositor can be accessed with outputs().\n
     * Add an already initialized output is a no-op.
     *
     * @param output One of the outputs available on the seat (seat()) through the LSeat::outputs() method.
     */
    bool addOutput(LOutput *output);

    /*!
     * Removes the output of the compositor, stopping its thread and rendering loop.
     *
     * @param output One of the outputs previously added to the compositor, accessible from outputs(). Removing an output that has not been added to the compositor is a no-op.
     */
    void removeOutput(LOutput *output);

    /*!
     * List of all surfaces created by clients.\n
     * The surfaces in the list respect the stacking order of their roles/protocols.\n
     * Use the LClient::surfaces() method to access surfaces created by a specific client.
     */
    const list<LSurface*>&surfaces() const;

    /*!
     * List of all initialized outputs with the addOutput() method.\n
     * @note This list only contains initialized outputs. To get a list of all outputs avaliable use LSeat::outputs() instead.
     */
    const list<LOutput*>&outputs() const;

    /*!
     * List of clients connected to the compositor.
     */
    const list<LClient*>&clients() const;

    /*!
     * Returns a new positive integer number each time is called, incrementally.
     */
    static UInt32 nextSerial();

    /*!
     * Main EGL display created by the graphics backend.
     */
    static EGLDisplay eglDisplay();

    /*!
     * Main EGL context created by the graphics backend.
     */
    static EGLContext eglContext();

    /*!
     * Flush all pending client events.
     */
    static void flushClients();

    /*!
      * Gets the LClient of a native Wayland wl_client resource.
      *
      * @returns The LClient instance for a **wl_client** resource or nullptr if not found.
      */
    LClient *getClientFromNativeResource(wl_client *client);

    /*!
      * ID of the main thread.
      * The main thread handles the Wayland and input backend event loop, while each output runs on its own thread.
      */
    thread::id mainThreadId() const;

    LPRIVATE_IMP(LCompositor)
};

#endif // LCOMPOSITOR_H
