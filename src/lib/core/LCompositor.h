#ifndef LCOMPOSITOR_H
#define LCOMPOSITOR_H

#include <LNamespaces.h>
#include <LFactoryObject.h>
#include <filesystem>
#include <thread>
#include <vector>
#include <list>

/**
 * @brief Louvre's core and resources factory.
 *
 * The LCompositor class manages the main event loop, backend systems, and provides virtual constructors
 * for Louvre objects with interfaces you can override.
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

        /// Idle state during a session (TTY) switch
        Paused,

        /// Changing from paused to initialized
        Resuming
    };

    /**
     * @brief Constructor of the LCompositor class.
     */
    LCompositor() noexcept;

    /**
     * @brief Destructor of the LCompositor class.
     */
    virtual ~LCompositor();

    LCLASS_NO_COPY(LCompositor)

    /**
     * @brief Notifies a successful compositor initialization.
     *
     * Use this event to handle the successful initialization of the compositor after calling start().
     * Here you should perform initial configuration tasks, such as setting up outputs, as demonstrated in the default implementation.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp initialized
     */
    virtual void initialized();

    /**
     * @brief Notifies the uninitialization of the compositor.
     *
     * This event is called right before the compositor is uninitialized when finish() is called. At this point, both the input and graphic
     * backends, along with other resources such as connected clients and initialized outputs, are still operational.
     * Use this opportunity to release any compositor-specific resources that you may have created.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp uninitialized
     */
    virtual void uninitialized();

    /**
     * @brief Notifies a successful cursor initialization.
     *
     * Use this event to handle the successful initialization of the cursor.
     * It is recommended to load cursor textures within this method.
     * The LXCursor class provides the LXCursor::loadXCursorB() method for loading pixmaps of XCursors available on the system.
     *
     * The default implementation includes a commented example demonstrating how to load XCursor pixmaps and assign them to the cursor.
     *
     * @par Default Implementation
     * @snippet LCompositorDefault.cpp cursorInitialized
     */
    virtual void cursorInitialized();

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
     * @brief Request to create an object.
     *
     * Certain classes in Louvre have an interface that can be overridden.
     * Instances of these classes can only be created from this virtual constructor when requested by the compositor.\n
     * Refer to LFactoryObject::Type to see all the classes that can be overridden.
     *
     * @param objectType Indicates the type of the class instance to return.
     * @param params An opaque data type that should be passed to the object's constructor.
     *
     * @return If `nullptr` is returned, Louvre will create an instance of the object using the base class.
     */
    virtual LFactoryObject *createObjectRequest(LFactoryObject::Type objectType, const void *params);

    /**
     * @brief Announce the anticipated destruction of an object.
     *
     * When an object's destructor is invoked, it is likely that associated resources are no longer available.\n
     * Using this event, however, provides an opportunity to access those resources.
     *
     * @warning Do not delete the object, Louvre deletes it later.
     *
     * @param object The object for which destruction is anticipated.
     */
    virtual void onAnticipatedObjectDestruction(LFactoryObject *object);

    /**
     * @brief Gets the current Louvre version.
     */
    static const LVersion &version();

    /**
     * @brief Creates and adds a global to the compositor.
     *
     * This method creates and adds a global to the compositor. Globals implemented by Louvre can be found in `<protocols/{protocol}/G{global}.h>`.
     *
     * Louvre automatically creates all the supported globals during compositor initialization. See LCompositor::createGlobalsRequest().
     *
     * @tparam Global The type of global to create. It should be a subclass of Louvre::LResource.
     * @param data Optional user data to associate with the global.
     * @return Pointer to the created LGlobal instance, which can be later removed with removeGlobal().
     */
    template<class Global>
    LGlobal *createGlobal(void *data = nullptr)
    {
        static_assert(std::is_base_of_v<Louvre::LResource, Global> == true);

        return globalCreate(Global::interface(),
                            Global::maxVersion(),
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
    void removeGlobal(LGlobal *global);

    /**
     * @brief Get the absolute path to the default Louvre assets directory.
     *
     * This path is automatically generated by Meson and typically points to `PREFIX/DATADIR/Louvre/assets`.
     *
     * @return The absolute path to the default Louvre assets directory.
     */
    const std::filesystem::path &defaultAssetsPath() const;

    /**
     * @brief Get the absolute path to the default Louvre backends directory.
     *
     * This path is automatically generated by Meson and typically points to `PREFIX/LIBDIR/Louvre/backends`.
     * Used when the **LOUVRE_BACKENDS_PATH** environment variable is unset.
     *
     * @return The absolute path to the default Louvre backends directory.
     */
    const std::filesystem::path &defaultBackendsPath() const;

    /**
     * @brief Loads a graphic backend (dynamic library).
     *
     * Use this method to load a custom graphic backend before calling start().
     * By default, Louvre loads the DRM backend which is tipically located at `/usr/local/lib/Louvre/backends/graphic/drm.so`.
     *
     * @note Instead of using this method, a more preferable approach is to let the user choose the graphical backend by
     *       configuring the **LOUVRE_BACKENDS_PATH** and **LOUVRE_GRAPHIC_BACKEND** environment variables.
     *
     * @param path Absolute path of the dynamic library.
     *
     * @return `true` if the backend is successfully loaded, `false` otherwise.
     */
    bool loadGraphicBackend(const std::filesystem::path &path);

    /**
     * @brief Checks if the graphic backend is initialized.
     *
     * Use this method to determine whether the graphic backend has been initialized after calling start().
     *
     * @return `true` if the graphic backend is initialized, `false` otherwise.
     */
    bool isGraphicBackendInitialized() const;

    /**
     * @brief Get the name of the default graphic backend.
     *
     * This name is used when the **LOUVRE_GRAPHIC_BACKEND** environment variable is unset.
     * It is automatically generated by Meson and defaults to `drm`.
     *
     * @return The name of the default graphic backend.
     */
    const std::string &defaultGraphicBackendName() const;

    /**
     * @brief Handle to the native context used by the graphic backend.
     *
     * This opaque handle is unique to each graphic backend.\n
     * In the case of the DRM backend, it returns a pointer to a [SRMCore](https://cuarzosoftware.github.io/SRM/group___s_r_m_core.html) struct.\n
     * In the case of the X11 backend, it returns a pointer to a [Display](https://www.x.org/releases/X11R7.6/doc/libX11/specs/libX11/libX11.html) struct.\n
     *
     * You can use this handle to configure specific aspects of each backend.
     */
    void *graphicBackendContextHandle() const;

    /**
     * @brief Get the ID of the current graphic backend.
     *
     * Each graphic backend is assigned a unique Louvre::UInt32 ID. You can use this method to retrieve the
     * ID of the current graphic backend in use.
     *
     * The IDs of graphic backends shipped with Louvre are listed in the Louvre::LGraphicBackendID enum.
     *
     * @return The ID of the graphic backend.
     */
    UInt32 graphicBackendId() const;

    /**
     * @brief Loads an input backend (dynamic library).
     *
     * Use this method to load a custom input backend before calling start().
     * By default, Louvre loads the Libinput backend which is tipically located at `/usr/local/lib/Louvre/backends/input/libinput.so`.
     *
     * @note Instead of using this method, a more preferable approach is to let the user choose the graphical backend by
     *       configuring the **LOUVRE_BACKENDS_PATH** and **LOUVRE_INPUT_BACKEND** environment variables.
     *
     * @param path Location of the backend's dynamic library.
     *
     * @return `true` if the backend is successfully loaded, `false` otherwise.
     */
    bool loadInputBackend(const std::filesystem::path &path);

    /**
     * @brief Checks if the input backend is initialized.
     *
     * Use this method to determine whether the input backend has been initialized after calling start().
     *
     * @return `true` if the input backend is initialized, `false` otherwise.
     */
    bool isInputBackendInitialized() const;

    /**
     * @brief Get the name of the default input backend.
     *
     * This name is used when the **LOUVRE_INPUT_BACKEND** environment variable is unset.
     * It is automatically generated by Meson and defaults to `libinput`.
     *
     * @return The name of the default input backend.
     */
    const std::string &defaultInputBackendName() const;

    /**
     * @brief Handle to the native context used by the input backend.
     *
     * This opaque handle is unique to each input backend.\n
     * In the case of the Libinput backend, it returns a pointer to a [libinput](https://wayland.freedesktop.org/libinput/doc/latest/api/structlibinput.html) struct.\n
     * In the case of the X11 backend, it returns a pointer to a [Display](https://www.x.org/releases/X11R7.6/doc/libX11/specs/libX11/libX11.html) struct.\n
     *
     * You can use this handle to configure specific aspects of each backend.
     */
    void *inputBackendContextHandle() const;

    /**
     * @brief Get the ID of the current input backend.
     *
     * Each input backend is assigned a unique Louvre::UInt32 ID. You can use this method to retrieve the
     * ID of the current input backend in use.
     *
     * The IDs of input backends shipped with Louvre are listed in the Louvre::LInputBackendID enum.
     *
     * @return The ID of the input backend.
     */
    UInt32 inputBackendId() const;

    /**
     * @brief Gets the current compositor state.
     *
     * Use this method to retrieve the current state of the compositor.
     *
     * @see LCompositor::CompositorState enum for possible state values.
     *
     * @return The current compositor state.
     */
    CompositorState state() const;

    /**
     * @brief Starts the event loop and backends.
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
     * You can also get a pollable file descriptor with fd().
     *
     * @param msTimeout Milliseconds to wait before an event occurs.
     *        Setting it to 0 disables the timeout, and setting it to -1 makes it wait indefinitely until an event occurs.
     *
     * @return 1 on success and 0 on failure.
     */
    Int32 processLoop(Int32 msTimeout);

    /**
     * @brief Get a pollable file descriptor of the main event loop.
     *
     * @return The pollable file descriptor.
     */
    Int32 fd() const;

    /**
     * @brief Ends and uninitializes the compositor.
     */
    void finish();

    /**
     * @brief Get the native `wl_display` used by the compositor.
     *
     * @return The native `wl_display`.
     */
    static wl_display *display();

    /**
     * @brief Add a pollable file descriptor to the compositor's event loop.
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
    LCursor *cursor() const;

    /**
     * @brief Gets the compositor seat.
     *
     * The seat provides access to the LPointer, LKeyboard, LTouch, and LOutput instances.
     *
     * @return A pointer to the LSeat instance.
     */
    LSeat *seat() const;

    /**
     * @brief Provides access to the session lock manager.
     *
     * The session lock manager allows you to handle client requests to lock the user session and display arbitrary content.
     *
     * @return A pointer to the LSessionLockManager instance.
     */
    LSessionLockManager *sessionLockManager() const noexcept;

    /**
     * @brief Schedule a new rendering frame for each output in the compositor.
     *
     * This method schedules a new rendering frame by calling the LOutput::repaint() method for all initialized outputs.
     */
    void repaintAllOutputs();

    /**
     * @brief Initializes the specified output.
     *
     * This method initializes the specified output for rendering, allowing you to schedule rendering frames
     * using the LOutput::repaint() method. The list of initialized outputs can be accessed with the outputs() method.
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
     * This method uninitializes and removes the specified output from the compositor, stopping its thread and rendering loop.
     *
     * @note Removing an output that has not been added to the compositor is a no-op.
     *
     * @warning Calling this method from the same rendering thread of the output (whithin any of its `...GL()` events) has no effect.
     *          Doing so would lead to a deadlock, so Louvre simply ignores the request.
     *
     * @param output The output to remove, previously added to the compositor.
     */
    void removeOutput(LOutput *output);

    /**
     * @brief Get a list of all surfaces created by clients.
     *
     * This method returns a list of all surfaces created by clients, respecting the stacking order of their roles/protocols.
     * To access surfaces created by a specific client, use the LClient::surfaces() method instead.
     *
     * @return A list of LSurface objects representing the surfaces.
     */
    const std::list<LSurface*> &surfaces() const;

    /**
     * @brief Get a list of all initialized outputs.
     *
     * This method returns a list of all outputs that have been initialized using the addOutput() method.
     *
     * @note This list only contains initialized outputs. To get a list of all available outputs, use LSeat::outputs() instead.
     *
     * @return A list of LOutput objects representing the initialized outputs.
     */
    const std::vector<LOutput*> &outputs() const;

    /**
     * @brief Get a list of clients connected to the compositor.
     *
     * This method returns a list of clients that are currently connected to the compositor.
     *
     * @return A list of LClient objects representing the connected clients.
     */
    const std::vector<LClient*> &clients() const;

    /**
     * @brief Get the main EGL display created by the graphic backend.
     *
     * This method returns the main EGL display created by the graphic backend.
     *
     * @return The main EGL display.
     */
    static EGLDisplay eglDisplay();

    /**
     * @brief Get the main EGL context created by the graphic backend.
     *
     * This method returns the main EGL context created by the graphic backend.
     *
     * @return The main EGL context.
     */
    static EGLContext eglContext();

    /**
     * @brief Flush all pending client events.
     *
     * This method immediatly flushes all pending client events.
     */
    static void flushClients();

    /**
      * @brief Gets the LClient of a native Wayland `wl_client` resource.
      *
      * @returns The LClient instance for a `wl_client` resource or `nullptr` if not found.
      */
    LClient *getClientFromNativeResource(wl_client *client);

    /**
     * @brief Identifier of the main thread.
     *
     * This ID corresponds to the primary thread responsible for managing the Wayland and input backend event loops,
     * while individual output operations are performed on separate threads.
     *
     * @return The identifier of the main thread.
     */
    std::thread::id mainThreadId() const;

    LPRIVATE_IMP_UNIQUE(LCompositor)

    LGlobal *globalCreate(const wl_interface *interface, Int32 version, void *data, wl_global_bind_func_t bind);

};

#endif // LCOMPOSITOR_H
