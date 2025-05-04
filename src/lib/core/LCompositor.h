#ifndef LCOMPOSITOR_H
#define LCOMPOSITOR_H

#include <LNamespaces.h>
#include <LFactoryObject.h>
#include <LLayout.h>
#include <filesystem>
#include <thread>
#include <unordered_set>
#include <vector>
#include <list>

/**
 * @brief Louvre's core and objects factory.
 *
 * The LCompositor class manages the main event loop, backend systems, and provides virtual constructors
 * and destructors for Louvre objects with interfaces you can override.
 *
 * @see LFactoryObject
 * @see createObjectRequest()
 * @see onAnticipatedObjectDestruction()
 */
class Louvre::LCompositor
{
public:

    /**
     * @brief Compositor state.
     *
     * @see state().
     */
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

        /// Idle state during a session (TTY) switch
        Paused,

        /// Changing from paused to initialized
        Resuming
    };

    /**
     * @brief Constructor of the LCompositor class.
     */
    LCompositor() noexcept;

    LCLASS_NO_COPY(LCompositor)

    /**
     * @brief Destructor of the LCompositor class.
     */
    virtual ~LCompositor();

    /**
     * @brief Gets the current Louvre version.
     */
    static const LVersion &version() noexcept;

    /**
     * @brief Gets the current compositor state.
     */
    CompositorState state() const noexcept;

    /**
     * @brief Starts the main event loop and backends.
     *
     * After successful initialization, notified with the initialized() event,
     * the compositor can receive connections from Wayland clients and initialize
     * output rendering threads using the addOutput() method.
     *
     * @return `false` on failure and `true` otherwise.
     */
    bool start();

    /**
     * @brief Process the compositor's main event loop.
     *
     * @note You can also get a pollable file descriptor with fd().
     *
     * @param msTimeout Milliseconds to wait before an event occurs.
     *        Setting it to 0 disables the timeout, and setting it to -1 makes it wait indefinitely until an event occurs.
     *
     * @return 1 on success and 0 on failure.
     */
    Int32 processLoop(Int32 msTimeout);

    /**
     * @brief Gets a pollable file descriptor of the main event loop.
     */
    Int32 fd() const noexcept;

    /**
     * @brief Uninitializes the compositor.
     *
     * @see uninitialized().
     */
    void finish() noexcept;

    /**
     * @brief Gets the native `wl_display` used by the compositor.
     *
     * @return The native `wl_display`.
     */
    static wl_display *display() noexcept;

    /**
     * @brief Adds a pollable file descriptor listener to the main event loop.
     *
     * @param fd The file descriptor to be added.
     * @param userData User data to pass to the callback function.
     * @param callback The callback function to handle events on the file descriptor.
     * @param flags Flags to specify the type of event to listen for (e.g., WL_EVENT_READABLE).
     *
     * @note If the compositor is suspended, events are queued and will be dispatched once it is resumed.
     *
     * @return The wl_event_source associated with the added file descriptor.
     */
    static wl_event_source *addFdListener(int fd, void *userData, int(*callback)(int,unsigned int,void*), UInt32 flags = WL_EVENT_READABLE);

    /**
     * @brief Removes a previously added file descriptor from the compositor's event loop.
     *
     * @param source The wl_event_source to remove.
     */
    static void removeFdListener(wl_event_source *source);

    /**
     * @brief Gets the compositor cursor.
     *
     * This method must be accessed within or after the initialized() or cursorInitialized() events.
     * If the cursor has not yet been initialized, this method returns `nullptr`.
     *
     * @return A pointer to the LCursor instance or `nullptr` if not yet initialized.
     */
    LCursor *cursor() const noexcept;

    /**
     * @brief Gets the compositor seat.
     *
     * The seat provides access to the LPointer, LKeyboard, LTouch, and LOutput instances.
     *
     * @return A pointer to the LSeat instance.
     */
    LSeat *seat() const noexcept;

    /**
     * @brief Provides access to the session lock manager.
     *
     * The session lock manager allows you to handle client requests to lock the user session and display arbitrary content.
     */
    LSessionLockManager *sessionLockManager() const noexcept;

    /**
     * @brief Provides access to the activation token manager.
     *
     * The activation token manager allows you to handle client requests to activate surfaces of other clients.
     */
    LActivationTokenManager *activationTokenManager() const noexcept;

    /**
     * @brief Gets a list of all surfaces created by clients.
     *
     * This method returns a list of all surfaces created by clients, respecting the stacking order of their roles protocols.\n
     * To access surfaces from a specific layer, used layer() instead.
     *
     * @return A list of LSurface objects representing the surfaces.
     */
    const std::list<LSurface*> &surfaces() const noexcept;

    /**
     * @brief Retrieves the list of surfaces within a specific layer.
     *
     * @note The layers lists follow the same order as surfaces().
     *
     * @param layer The layer index to retrieve. See @ref LSurfaceLayer.
     * @return A reference to a list of pointers to LSurface objects within the specified layer.
     */
    const std::list<LSurface*> &layer(LSurfaceLayer layer) const noexcept;

    /**
     * @brief Gets a list of clients connected to the compositor.
     *
     * This method returns a list of clients that are currently connected to the compositor.
     *
     * @return A list of LClient objects representing the connected clients.
     */
    const std::vector<LClient*> &clients() const noexcept;

    /**
     * @brief Flush all pending client events.
     *
     * This method immediatly flushes all pending client events.
     */
    static void flushClients() noexcept;

    /**
      * @brief Gets the LClient of a native Wayland `wl_client` resource.
      *
      * @returns The LClient instance for a `wl_client` resource or `nullptr` if not found.
      */
    LClient *getClientFromNativeResource(const wl_client *client) noexcept;

    /**
     * @brief Gets a vector of all initialized outputs.
     *
     * This method returns a vector of all outputs that have been initialized using the addOutput() method.
     *
     * @note This vector only contains initialized outputs. To get all available outputs, use LSeat::outputs() instead.
     */
    const std::vector<LOutput*> &outputs() const noexcept;

    /**
     * @brief Initializes the specified output.
     *
     * This method creates a new rendering thread and initializes the specified output. Once initialized,
     * the LOutput::initializeGL() event is triggered, and LOutput::repaint() can be used to unlock the
     * rendering thread loop, which triggers LOutput::paintGL() events.
     *
     * This adds the output to the outputs() vector.
     *
     * @note Adding an already initialized output is a no-op.
     *
     * @param output The output to initialize, obtained from LSeat::outputs().
     *
     * @return `true` on success, `false` on failure.
     */
    bool addOutput(LOutput *output);

    /**
     * @brief Uninitializes the specified output.
     *
     * This method uninitializes and removes the specified output from the compositor, stopping its thread, rendering loop
     * and triggering the LOutput::uninitializeGL() event.
     *
     * @note Removing an output that has not been added to the compositor is a no-op.
     *
     * @warning Calling this method from the same rendering thread of the output (whithin any of its `...GL()` events) has no effect.
     *          Doing so would lead to a deadlock, so Louvre simply ignores the request.
     *
     * @param output The output to remove, previously added to the compositor. See outputs().
     */
    void removeOutput(LOutput *output);

    /**
     * @brief Unlocks the rendering thread of all initialized outputs.
     *
     * This method schedules a new rendering frame by calling the LOutput::repaint() method to all initialized outputs.
     */
    void repaintAllOutputs() noexcept;

    /**
     * @brief Searches for the most intersected output
     *
     * Checks which output within the global compositor space intersects the given rectangle the most.
     *
     * @param rect The rectangle to check in compositor-global coordinates.
     * @param initializedOnly If `true`, only considers initialized outputs. Default is `true`.
     * @return A pointer to the most intersected LOutput, or `nullptr` if no output is found.
     */
    LOutput *mostIntersectedOutput(const LRect &rect, bool initializedOnly = true) const noexcept;

    /**
     * @brief Identifier of the main thread.
     *
     * This ID corresponds to the primary thread responsible for managing the Wayland and input backend event loops,
     * while individual output operations are performed on separate threads.
     *
     * @return The identifier of the main thread.
     */
    std::thread::id mainThreadId() const noexcept;

    /**
     * @brief Gets the main EGL display created by the graphic backend.
     */
    static EGLDisplay eglDisplay() noexcept;

    /**
     * @brief Gets the main EGL context created by the graphic backend.
     */
    static EGLContext eglContext() noexcept;

    /**
     * @brief Gets the absolute path to the default Louvre assets directory.
     *
     * This path is automatically generated by Meson and should point to `PREFIX/DATADIR/Louvre/assets`.
     *
     * @return The absolute path to the default Louvre assets directory.
     */
    const std::filesystem::path &defaultAssetsPath() const noexcept;

    /**
     * @brief Gets the absolute path to the default Louvre backends directory.
     *
     * This path is automatically generated by Meson and should point to `PREFIX/LIBDIR/Louvre/backends`.\n
     * Used when the **LOUVRE_BACKENDS_PATH** environment variable is unset.
     *
     * @return The absolute path to the default Louvre backends directory.
     */
    const std::filesystem::path &defaultBackendsPath() const noexcept;

    /**
     * @brief Loads a graphic backend (dynamic library).
     *
     * Use this method to load a custom graphic backend before calling start().\n
     * By default, Louvre tries to load the Wayland backend if **WAYLAND_DISPLAY** is set, and then the DRM
     * backend if fails.
     *
     * @note Instead of using this method, a more preferable approach is to let the user choose the graphical backend by
     *       configuring the **LOUVRE_BACKENDS_PATH** and **LOUVRE_GRAPHIC_BACKEND** environment variables.
     *
     * @param path Absolute path of the dynamic library.
     *
     * @return `true` if the backend is successfully loaded, `false` otherwise.
     */
    bool loadGraphicBackend(const std::filesystem::path &path) noexcept;

    /**
     * @brief Checks if the graphic backend is initialized.
     *
     * Use this method to determine whether the graphic backend has been initialized after calling start().
     *
     * @return `true` if the graphic backend is initialized, `false` otherwise.
     */
    bool isGraphicBackendInitialized() const noexcept;

    /**
     * @brief Gets the name of the default graphic backend.
     *
     * This name is used when the **LOUVRE_GRAPHIC_BACKEND** environment variable is unset.\n
     * It is automatically generated by Meson and defaults to `drm`.
     *
     * @return The name of the default graphic backend.
     */
    const std::string &defaultGraphicBackendName() const noexcept;

    /**
     * @brief Handle to the native context used by the graphic backend.
     *
     * This opaque handle is unique to each graphic backend.\n
     * - In the case of the DRM backend, it returns a pointer to a [SRMCore](https://cuarzosoftware.github.io/SRM/group___s_r_m_core.html) struct.
     * - In the case of the Wayland backend, it returns a pointer to a [wl_display](https://wayland.freedesktop.org/docs/html/apb.html#Client-classwl__display) struct.
     *
     * You can use this handle to configure specific aspects of each backend.
     */
    void *graphicBackendContextHandle() const noexcept;

    /**
     * @brief Gets the ID of the current graphic backend.
     *
     * Each graphic backend is assigned a unique @ref UInt32 ID. You can use this method to retrieve the
     * ID of the current graphic backend in use.
     *
     * The IDs of graphic backends shipped with Louvre are listed in the @ref LGraphicBackendID enum.
     *
     * @return The ID of the graphic backend.
     */
    UInt32 graphicBackendId() const noexcept;

    /**
     * @brief Loads an input backend (dynamic library).
     *
     * Use this method to load a custom graphic backend before calling start().\n
     * By default, Louvre tries to load the Wayland backend if **WAYLAND_DISPLAY** is set, and then the Libinput
     * backend if fails.
     *
     * @note Instead of using this method, a more preferable approach is to let the user choose the graphical backend by
     *       configuring the **LOUVRE_BACKENDS_PATH** and **LOUVRE_INPUT_BACKEND** environment variables.
     *
     * @param path Location of the backend's dynamic library.
     *
     * @return `true` if the backend is successfully loaded, `false` otherwise.
     */
    bool loadInputBackend(const std::filesystem::path &path) noexcept;

    /**
     * @brief Checks if the input backend is initialized.
     *
     * Use this method to determine whether the input backend has been initialized after calling start().
     *
     * @return `true` if the input backend is initialized, `false` otherwise.
     */
    bool isInputBackendInitialized() const noexcept;

    /**
     * @brief Gets the name of the default input backend.
     *
     * This name is used when the **LOUVRE_INPUT_BACKEND** environment variable is unset.\n
     * It is automatically generated by Meson and defaults to `libinput`.
     *
     * @return The name of the default input backend.
     */
    const std::string &defaultInputBackendName() const noexcept;

    /**
     * @brief Handle to the native context used by the input backend.
     *
     * This opaque handle is unique to each input backend.\n
     * - In the case of the Libinput backend, it returns a pointer to a [libinput](https://wayland.freedesktop.org/libinput/doc/latest/api/structlibinput.html) struct.
     * - In the case of the Wayland backend, it returns a pointer to a [wl_display](https://wayland.freedesktop.org/docs/html/apb.html#Client-classwl__display) struct.
     *
     * You can use this handle to configure specific aspects of each backend.
     */
    void *inputBackendContextHandle() const noexcept;

    /**
     * @brief Gets the ID of the current input backend.
     *
     * Each input backend is assigned a unique Louvre::UInt32 ID. You can use this method to retrieve the
     * ID of the current input backend in use.
     *
     * The IDs of input backends shipped with Louvre are listed in the Louvre::LInputBackendID enum.
     *
     * @return The ID of the input backend.
     */
    UInt32 inputBackendId() const noexcept;

    /**
     * @brief Creates and adds a global to the compositor.
     *
     * This method creates and adds a global to the compositor. Globals implemented by Louvre can be found in `<protocols/{protocol}/G{global}.h>`.
     *
     * Louvre automatically creates all the supported globals during compositor initialization. See LCompositor::createGlobalsRequest().
     *
     * @tparam Global The type of global to create. It should be a subclass of Louvre::LResource.
     * @param version The maximum supported interface version, constrained by Louvre’s implemented version.
     *                If <= 0, Louvre’s highest supported version is used instead.
     * @param data User data to assign to the `wl_resource`.
     * @return Pointer to the created LGlobal instance, which can be later removed with removeGlobal().
     */
    template<class Global>
    LGlobal *createGlobal(Int32 version = 0, void *data = nullptr) noexcept
    {
        static_assert(std::is_base_of_v<Louvre::LResource, Global> == true);

        return globalCreate(Global::interface(),
                            version <= 0 ? Global::maxVersion() : std::min(version, Global::maxVersion()),
                            data,
                            &Global::bind);
    }

    /**
     * @brief Safely removes a global.
     *
     * This method performs a lazy removal of an LGlobal created with createGlobal(), ensuring clients are notified before it is destroyed.
     *
     * @note Calling LCompositor::removeGlobal() isn't mandatory as globals are automatically destroyed during the compositor's uninitialization.
     *
     * @warning After calling this method, the LGlobal instance must no longer be used as it is going to be destroyed.
     *
     * @param global Pointer to the LGlobal instance to remove.
     */
    void removeGlobal(LGlobal *global) noexcept;

    /**
     * @name Posix Signals
     */

    ///@{

    /**
     * @brief Registers a POSIX signal for synchronous handling.
     *
     * This method registers POSIX signals for synchronous dispatching by the main event loop,
     * as explained in [wl_event_loop_add_signal](https://wayland.freedesktop.org/docs/html/apc.html#Server-structwl__event__source_1a1706e2490502a95f24ccb59cbae3e2f8).
     *
     * All registered signals can be handled in onPosixSignal().
     *
     * Louvre automatically disables the signal in all rendering threads, but this does not apply to user-created threads.
     *
     * @note If called from a rendering thread, the signal addition will be scheduled for the next main loop iteration.
     *
     * @see removePosixSignal()
     * @see posixSignals()
     *
     * @return `true` if successfully added, `false` if the signal is already registered or
     *         if attempted while the compositor is uninitialized.
     */
    bool addPosixSignal(int signal) noexcept;

    /**
     * @brief Removes a registered POSIX signal.
     *
     * Unregisters and unblocks a previously added POSIX signal, preventing it from being handled by the main event loop.
     *
     * @note If called from a rendering thread, the signal removal will be scheduled for the next main loop iteration.
     *
     * @note Signals are automatically removed after the compositor is uninitialized.
     *
     * @return `true` if the signal was successfully removed, `false` if it was not registered.
     */
    bool removePosixSignal(int signal) noexcept;

    /**
     * @brief Retrieves the set of registered POSIX signals.
     *
     * Returns the set of POSIX signals currently registered via addPosixSignal().
     */
    const std::unordered_set<int> &posixSignals() const noexcept;

    ///@}

    /**
     * @name Virtual Methods
     */

    ///@{

    /**
     * @brief Notifies a successful compositor initialization.
     *
     * Use this event to handle the successful initialization of the compositor after calling start().\n
     * Here you should perform initial configuration tasks, such as setting up outputs, as demonstrated in the default implementation.
     *
     * @warning Avoid interacting with any LFactoryObjects (e.g., LPointer, LKeyboard, LTouch, etc.)
     *          or creating LTextures before this event. These objects or the input/graphic backends
     *          might be partially initialized, and interacting with them could cause a segmentation fault.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp initialized
     */
    virtual void initialized();

    /**
     * @brief Notifies the uninitialization of the compositor.
     *
     * This event is triggered right before the compositor is uninitialized when finish() is called. At this point, both the input and graphic
     * backends, along with other resources such as connected clients and initialized outputs, are still operational.\n
     * Use this opportunity to release any compositor-specific resources that you may have created.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp uninitialized
     */
    virtual void uninitialized();

    /**
     * @brief Request to create a specific LFactoryObject.
     *
     * Certain classes in Louvre have an interface that can be overridden.\n
     * Instances of these classes can only be created from this virtual constructor when requested by the compositor.\n
     * Refer to @ref LFactoryObject::Type to see all the classes that can be overridden.
     *
     * @param objectType Indicates the type of the class instance to return.
     * @param params An opaque data type that should be passed to the object's constructor.
     *
     * @return If `nullptr` is returned, Louvre will create an instance of the object using the base class.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createObjectRequest
     */
    virtual LFactoryObject *createObjectRequest(LFactoryObject::Type objectType, const void *params);

    /**
     * @brief Announce the anticipated destruction of an LFactoryObject.
     *
     * When an LFactoryObject destructor is invoked, it is likely that associated resources are no longer available.\n
     * Using this event, however, provides an opportunity to access those resources.
     *
     * @warning Do not delete the object, Louvre deletes it later.
     *
     * @param object The object for which destruction is anticipated.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp onAnticipatedObjectDestruction
     */
    virtual void onAnticipatedObjectDestruction(LFactoryObject *object);

    /**
     * @brief Wayland globals initialization.
     *
     * Override this method if you want to remove or add custom Wayland globals.
     *
     * @see createGlobal()
     * @see removeGlobal()
     *
     * @return `true` on success, `false` on failure (prevents the compositor from starting).
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp createGlobalsRequest
     */
    virtual bool createGlobalsRequest();

    /**
     * @brief Globals filter.
     *
     * Filter to prevent clients from using specific protocol globals.\n
     * It is recommended to use this method to allow only well-known clients to bind to
     * protocols such as Session Lock, Wlr Screen Copy, and others.
     *
     * See LClient::credentials() and LGlobal::isType().
     *
     * @param client The client attempting to bind to the global.
     * @param global The protocol global the client is attempting to bind to.
     * @returns `true` to allow the client, `false` to deny it.
     *
     * @par Default Implementation
     *
     * The default implementation allows all clients to bind to all globals.
     *
     * @snippet LCompositorDefault.cpp globalsFilter
     */
    virtual bool globalsFilter(LClient *client, LGlobal *global);

    /**
     * @brief Handles POSIX signals.
     *
     * Centralized function for managing all POSIX signals registered via addPosixSignal().
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp onPosixSignal
     */
    virtual void onPosixSignal(int signal);

    ///@}

    LPRIVATE_IMP_UNIQUE(LCompositor)

    LGlobal *globalCreate(const wl_interface *interface, Int32 version, void *data, wl_global_bind_func_t bind);
};

#endif // LCOMPOSITOR_H
